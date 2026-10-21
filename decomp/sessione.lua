-- Sessione guidata del Z80 principale con finestre di traccia del debugger.
-- Uso (da decomp/): mame solomon ... -debug -debugger none -autoboot_script sessione.lua
-- Variabili d'ambiente:
--   SESS_NOME      prefisso dei file in out/ (predefinito "sessione")
--   SESS_ROOM      stanza forzata 0-based (tap su c243 fra i frame 1500 e 1950);
--                  vuota = nessun tap
--   SESS_TRACE_DA  inizio delle finestre di traccia, separati da virgola (es. 7400,19400)
--   SESS_TRACE_A   fine delle finestre, stessa quantità; l'ultima fine chiude la sessione.
--                  La prima finestra crea out/<nome>_trace.txt, le altre vi accodano.
--   SESS_INPUT     nessuno | casuale | file.lua (il file ritorna una lista di
--                  {frame, campo, valore}, campo = nome del campo di :P1 o :SYSTEM)
--   SESS_SEED      seme degli input casuali (predefinito 1)
--   SESS_CASUALE_DA frame da cui partono gli input casuali (predefinito 1900)
--   SESS_MONETA    1 = moneta e start ai frame 1510-1580 (predefinito), 0 = nessuna
--                  moneta (attract, oppure -playback dove gli input vengono dal file)
--   SESS_POKE      forzature di RAM: elenco «frame:addr:val» separati da virgola
--                  (esadecimali con 0x ammessi, es. 2250:0xc19d:0); scritte con
--                  mem:write_u8 a fine frame, stampa «POKE <frame> <addr> <val>»
--   SESS_POSTA     poste di attività: elenco «frame:codice»; al frame detto scrive
--                  il codice nel primo slot libero (= 0) di c000-c03f e stampa
--                  «POSTA <frame> <slot> <codice>» (slot assente se la tabella è piena)
-- Stampa «FASE <frame> <codici>» a ogni cambio dei codici non nulli in c000-c03f
-- (in ordine di slot) e scrive out/<nome>_e603.txt («PC valore conteggio», PC del tap
-- = PC dopo l'istruzione, come e603.lua).
local nome = os.getenv("SESS_NOME") or "sessione"
local stanza = os.getenv("SESS_ROOM")
if stanza == "" then stanza = nil end
if stanza then stanza = tonumber(stanza) end
local modo = os.getenv("SESS_INPUT") or "nessuno"
local seme = tonumber(os.getenv("SESS_SEED") or "1")
local casuale_da = tonumber(os.getenv("SESS_CASUALE_DA") or "1900")
local con_moneta = (os.getenv("SESS_MONETA") or "1") ~= "0"

local function lista(testo)
  local t = {}
  for x in string.gmatch(testo or "", "%d+") do t[#t + 1] = tonumber(x) end
  return t
end
-- elenco «a:b[:c],...» -> lista di tabelle di numeri (tonumber accetta 0x..)
local function inneschi(testo)
  local t = {}
  for voce in string.gmatch(testo or "", "[^,%s]+") do
    local v = {}
    for x in string.gmatch(voce, "[^:]+") do v[#v + 1] = assert(tonumber(x), "numero non valido: " .. x) end
    t[#t + 1] = v
  end
  return t
end
local pokes = inneschi(os.getenv("SESS_POKE"))
local poste = inneschi(os.getenv("SESS_POSTA"))
local trace_da = lista(os.getenv("SESS_TRACE_DA"))
local trace_a = lista(os.getenv("SESS_TRACE_A"))
assert(#trace_da == #trace_a and #trace_da > 0, "SESS_TRACE_DA e SESS_TRACE_A: stesse quantità, almeno una")
local fine = trace_a[#trace_a]
local file_traccia = string.format("out/%s_trace.txt", nome)

local frames = 0
local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local dbg = manager.machine.debugger
local porte = manager.machine.ioport.ports

local function campo(n)
  for _, p in ipairs({ ":P1", ":SYSTEM", ":P2", ":DSW1", ":DSW2" }) do
    local porta = porte[p]
    if porta and porta.fields[n] then return porta.fields[n] end
  end
  error("campo di ingresso sconosciuto: " .. n)
end
local moneta = campo("Coin 1")
local start = campo("1 Player Start")
local nomi = { "P1 Left", "P1 Right", "P1 Up", "P1 Down", "P1 Button 1", "P1 Button 2" }
local campi = {}
for i, n in ipairs(nomi) do campi[i] = campo(n) end
local premuto = nil
math.randomseed(seme)

-- input da file: indice per frame
local da_file = {}
if modo ~= "nessuno" and modo ~= "casuale" then
  local voci = dofile(modo)
  for _, v in ipairs(voci) do
    local f = v[1]
    da_file[f] = da_file[f] or {}
    table.insert(da_file[f], { campo(v[2]), v[3] })
  end
end

-- tap su e603 (handle globali: altrimenti il garbage collector li toglie)
local conteggi = {}
TAP_E603 = mem:install_read_tap(0xe603, 0xe603, "e603_sess", function(offset, data, mask)
  local pc = cpu.state["PC"].value
  local per_pc = conteggi[pc]
  if not per_pc then per_pc = {} conteggi[pc] = per_pc end
  per_pc[data & 0xff] = (per_pc[data & 0xff] or 0) + 1
end)
TAP_STANZA = nil

local function scrivi_e603()
  local chiavi = {}
  for pc, per_pc in pairs(conteggi) do
    for v, n in pairs(per_pc) do chiavi[#chiavi + 1] = { pc, v, n } end
  end
  table.sort(chiavi, function(x, y) return x[1] < y[1] or (x[1] == y[1] and x[2] < y[2]) end)
  local f = io.open(string.format("out/%s_e603.txt", nome), "w")
  for _, k in ipairs(chiavi) do f:write(string.format("%04X %02X %d\n", k[1], k[2], k[3])) end
  f:close()
end

local fase_prec = nil
local function controlla_fase()
  local t = {}
  for a = 0xc000, 0xc03f do
    local v = mem:read_u8(a)
    if v ~= 0 then t[#t + 1] = string.format("%02X", v) end
  end
  local s = table.concat(t, " ")
  if s ~= fase_prec then
    print(string.format("FASE %d %s", frames, s))
    fase_prec = s
  end
end

-- finestra che parte da 0: traccia aperta prima di avviare la CPU
if trace_da[1] == 0 then
  dbg:command(string.format("trace %s,0,noloop", file_traccia))
  print(string.format("TRACCIA APERTA al frame 0 (%s)", nome))
end
dbg:command("go")

emu.register_frame_done(function()
  frames = frames + 1
  if stanza and frames == 1500 then
    TAP_STANZA = mem:install_read_tap(0xc243, 0xc243, "stanza_sess", function(o, d, m) return stanza end)
  end
  if stanza and frames == 1950 and TAP_STANZA then TAP_STANZA:remove() TAP_STANZA = nil end
  if con_moneta then
    if frames == 1510 then moneta:set_value(1) end
    if frames == 1530 then moneta:set_value(0) end
    if frames == 1560 then start:set_value(1) end
    if frames == 1580 then start:set_value(0) end
  end
  if modo == "casuale" and frames >= casuale_da and frames < fine
     and (frames - casuale_da) % 8 == 0 then
    if premuto then premuto:set_value(0) end
    premuto = campi[math.random(#campi)]
    premuto:set_value(1)
  end
  if da_file[frames] then
    for _, v in ipairs(da_file[frames]) do v[1]:set_value(v[2]) end
  end
  for _, p in ipairs(pokes) do
    if p[1] == frames then
      mem:write_u8(p[2], p[3])
      print(string.format("POKE %d %04X %02X", frames, p[2], p[3]))
    end
  end
  for _, p in ipairs(poste) do
    if p[1] == frames then
      local slot = nil
      for a = 0xc000, 0xc03f do
        if mem:read_u8(a) == 0 then slot = a break end
      end
      if slot then
        mem:write_u8(slot, p[2])
        print(string.format("POSTA %d %02X %02X", frames, slot - 0xc000, p[2]))
      else
        print(string.format("POSTA %d piena %02X", frames, p[2]))
      end
    end
  end
  controlla_fase()
  for i = 1, #trace_da do
    if frames == trace_da[i] and frames > 0 then
      local pref = (i == 1) and "" or ">>"
      dbg:command(string.format("trace %s%s,0,noloop", pref, file_traccia))
      print(string.format("TRACCIA APERTA al frame %d (%s)", frames, nome))
    end
    if frames == trace_a[i] then
      dbg:command("trace off")
      print(string.format("TRACCIA CHIUSA al frame %d (%s)", frames, nome))
    end
  end
  if frames == fine then
    if premuto then premuto:set_value(0) end
    scrivi_e603()
    manager.machine:exit()
  end
end)
