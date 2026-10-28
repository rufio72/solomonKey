-- Stato prima/dopo di una routine del Z80 principale.
-- Variabili d'ambiente: SNAP_ENTRY (es. 0x05B3), SNAP_N (occorrenza, predefinito 1),
-- SNAP_DIR (predefinito out), SNAP_DA_FRAME (frame da cui armare il breakpoint,
-- predefinito 0: le occorrenze si contano da lì). Scrive <dir>/snap_<ENTRY>_prima.bin, _dopo.bin
-- (16384 byte c000-ffff) e _regs.txt. Le catture interrotte dalla NMI si scartano.
-- SNAP_TAG: se non vuoto i file diventano <dir>/snap_<ENTRY>_<TAG>_prima.bin ecc.
-- SNAP_INGRESSI: nessuno (predefinito) o traccia (moneta, start e input casuali
-- dal frame 1900 come trace.lua, con math.randomseed(1)).
-- SNAP_LIMITE: frame massimo senza cattura (predefinito 6000).
-- SNAP_COND: condizione del breakpoint d'ingresso nella sintassi del debugger
-- di MAME (es. a>=#10, decimale con #); vuota (predefinito) = nessuna. Le
-- occorrenze contate da SNAP_N sono solo quelle che la soddisfano.
-- SNAP_POKE, SNAP_POSTA: inneschi come SESS_POKE/SESS_POSTA di sessione.lua
-- («frame:addr:val» e «frame:codice», separati da virgola, 0x ammesso): forzature
-- di RAM e poste nel primo slot libero di c000-c03f al frame detto.
-- SNAP_ROOM: stanza forzata 0-based (tap in lettura su c243 fra i frame 1500
-- e 1950, come SESS_ROOM di sessione.lua); vuota = nessuna. Con SNAP_ROOM il
-- breakpoint si arma non prima del frame 1951, a tap tolto: altrimenti i dump
-- leggerebbero c243 attraverso il tap e una cattura a cavallo del frame 1950
-- avrebbe c243 diverso fra «prima» e «dopo».
-- SNAP_FORZA: forzature applicate all'occorrenza voluta, prima del dump
-- «prima» e della riga dei registri (brief 27). Voci separate da virgola:
-- ix+HH:VV e iy+HH:VV (byte in RAM a IX/IY + HH esadecimale), r:VV con r in
-- a, b, c, d, e, h, l (registro a 8 bit); VV esadecimale (0x ammesso). La
-- cattura resta originale: MAME esegue il codice vero dallo stato forzato.
local testo_entry = os.getenv("SNAP_ENTRY") or "0x05B3"
local entry = tonumber(testo_entry)
local voluta = tonumber(os.getenv("SNAP_N") or "1")
local dir = os.getenv("SNAP_DIR") or "out"
local da_frame = tonumber(os.getenv("SNAP_DA_FRAME") or "0")
local tag = os.getenv("SNAP_TAG") or ""
local ingressi = os.getenv("SNAP_INGRESSI") or "nessuno"
local limite = tonumber(os.getenv("SNAP_LIMITE") or "6000")
local cond = os.getenv("SNAP_COND") or ""
local function inneschi(testo)
  local t = {}
  for voce in string.gmatch(testo or "", "[^,%s]+") do
    local v = {}
    for x in string.gmatch(voce, "[^:]+") do v[#v + 1] = assert(tonumber(x), "numero non valido: " .. x) end
    t[#t + 1] = v
  end
  return t
end
local stanza = os.getenv("SNAP_ROOM") or ""
if stanza == "" then stanza = nil else stanza = assert(tonumber(stanza), "SNAP_ROOM non valida") end
local TAP_STANZA
local FINE_TAP = 1950
if stanza and da_frame <= FINE_TAP then da_frame = FINE_TAP + 1 end
-- SNAP_FORZA: elenco di { tipo, scostamento o registro, valore }
local forzature = {}
for voce in string.gmatch(os.getenv("SNAP_FORZA") or "", "[^,%s]+") do
  local base_r, off, val = string.match(voce, "^(i[xy])%+([%x]+):(%w+)$")
  if base_r then
    forzature[#forzature + 1] = { base_r, tonumber(off, 16), assert(tonumber(val, 16) or tonumber(val), "valore non valido: " .. voce) }
  else
    local r, v = string.match(voce, "^([abcdehl]):(%w+)$")
    assert(r, "forzatura non valida: " .. voce)
    forzature[#forzature + 1] = { r, 0, assert(tonumber(v, 16) or tonumber(v), "valore non valido: " .. voce) }
  end
end
local pokes = inneschi(os.getenv("SNAP_POKE"))
local poste = inneschi(os.getenv("SNAP_POSTA"))
local base = string.format("%s/snap_%s", dir, testo_entry)
if tag ~= "" then base = base .. "_" .. tag end
local testo_tag = (tag ~= "") and (" [" .. tag .. "]") or ""

local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local dbg = manager.machine.debugger
local REG = { "PC", "AF", "BC", "DE", "HL", "IX", "IY", "SP" }

local fase = "attesa"      -- attesa | dopo | fatto
local occorrenze = 0
local scartate = 0
local riga_prima, ritorno, sp_ritorno
local frames = 0

-- Ingressi «traccia»: stessa sequenza di trace.lua (righe 7-13 e 21-31).
local moneta, start, campi, premuto
if ingressi == "traccia" then
  local porte = manager.machine.ioport.ports
  moneta = porte[":SYSTEM"].fields["Coin 1"]
  start = porte[":SYSTEM"].fields["1 Player Start"]
  local nomi = { "P1 Left", "P1 Right", "P1 Up", "P1 Down", "P1 Button 1", "P1 Button 2" }
  campi = {}
  for i, n in ipairs(nomi) do campi[i] = porte[":P1"].fields[n] end
  math.randomseed(1)
end

local function muovi_ingressi()
  if ingressi ~= "traccia" then return end
  if frames == 1510 then moneta:set_value(1) end
  if frames == 1530 then moneta:set_value(0) end
  if frames == 1560 then start:set_value(1) end
  if frames == 1580 then start:set_value(0) end
  if frames >= 1900 and (frames - 1900) % 8 == 0 then
    if premuto then premuto:set_value(0) end
    premuto = campi[math.random(#campi)]
    premuto:set_value(1)
  end
end

local function applica_inneschi()
  for _, p in ipairs(pokes) do
    if p[1] == frames then
      mem:write_u8(p[2], p[3])
      print(string.format("POKE %d %04X %02X", frames, p[2], p[3]))
    end
  end
  for _, p in ipairs(poste) do
    if p[1] == frames then
      for a = 0xc000, 0xc03f do
        if mem:read_u8(a) == 0 then
          mem:write_u8(a, p[2])
          print(string.format("POSTA %d %02X %02X", frames, a - 0xc000, p[2]))
          break
        end
      end
    end
  end
end

local function applica_forzature()
  for _, f in ipairs(forzature) do
    if f[1] == "ix" or f[1] == "iy" then
      local a = (cpu.state[string.upper(f[1])].value + f[2]) & 0xffff
      mem:write_u8(a, f[3] & 0xff)
      print(string.format("FORZA %s+%02X (%04X) = %02X", f[1], f[2], a, f[3] & 0xff))
    else
      cpu.state[string.upper(f[1])].value = f[3] & 0xff
      print(string.format("FORZA %s = %02X", f[1], f[3] & 0xff))
    end
  end
end

local function registri(nome)
  local s = nome
  for _, r in ipairs(REG) do
    s = s .. string.format(" %s=%04X", r, cpu.state[r].value)
  end
  return s
end

local function scarica(nomefile)
  local f = io.open(nomefile, "wb")
  local t = {}
  for a = 0xc000, 0xffff do t[#t + 1] = string.char(mem:read_u8(a)) end
  f:write(table.concat(t))
  f:close()
end

local function arma_ingresso()
  dbg:command("bpclear")
  if cond ~= "" then
    dbg:command(string.format("bpset %X,%s", entry, cond))
  else
    dbg:command(string.format("bpset %X", entry))
  end
end

if da_frame == 0 then arma_ingresso() end
dbg:command("go")

emu.register_frame_done(function()
  frames = frames + 1
  if stanza and frames == 1500 then
    TAP_STANZA = mem:install_read_tap(0xc243, 0xc243, "stanza_snap", function(o, d, m) return stanza end)
  end
  if stanza and frames == FINE_TAP and TAP_STANZA then TAP_STANZA:remove() TAP_STANZA = nil end
  muovi_ingressi()
  applica_inneschi()
  if da_frame > 0 and frames == da_frame then arma_ingresso() end
  if frames >= limite and fase ~= "fatto" then
    print(string.format("SNAP: limite di %d frame raggiunto senza cattura", limite))
    fase = "fatto"
    manager.machine:exit()
  end
end)

emu.register_periodic(function()
  if fase == "fatto" or dbg.execution_state ~= "stop" then return end
  local pc = cpu.state["PC"].value
  if fase == "attesa" then
    if pc ~= entry then dbg:command("go") return end
    occorrenze = occorrenze + 1
    if occorrenze < voluta then dbg:command("go") return end
    applica_forzature()
    riga_prima = registri("prima")
    scarica(base .. "_prima.bin")
    local sp = cpu.state["SP"].value
    ritorno = mem:read_u16(sp)
    sp_ritorno = (sp + 2) & 0xffff
    dbg:command("bpclear")
    dbg:command(string.format("bpset %X,sp==%X", ritorno, sp_ritorno))
    dbg:command("bpset 66")
    fase = "dopo"
    dbg:command("go")
  elseif fase == "dopo" then
    if pc == 0x0066 then
      -- la NMI è entrata durante la routine: cattura contaminata
      scartate = scartate + 1
      print(string.format("SNAP: occorrenza %d scartata (NMI)", occorrenze))
      fase = "attesa"
      arma_ingresso()
      dbg:command("go")
      return
    end
    if pc ~= ritorno or cpu.state["SP"].value ~= sp_ritorno then
      dbg:command("go") return
    end
    scarica(base .. "_dopo.bin")
    local f = io.open(base .. "_regs.txt", "w")
    f:write(riga_prima .. "\n" .. registri("dopo") .. "\n")
    f:close()
    print(string.format("SNAP: catturata occorrenza %d di %s%s al frame %d, scartate %d",
      occorrenze, testo_entry, testo_tag, frames, scartate))
    fase = "fatto"
    dbg:command("bpclear")
    manager.machine:exit()
  end
end)
