-- Osservazione del Z80 audio (:audiocpu): comandi forzati nel latch, oracolo
-- delle scritture ai tre YM2149, scritture del latch e della RAM audio.
-- Uso (da decomp/): mame solomon ... -debug -debugger none -autoboot_script audio.lua
-- Variabili d'ambiente:
--   AUDIO_NOME      prefisso dei file in out/audio/ (predefinito "audio")
--   AUDIO_CODICI    codici da forzare, esadecimali, separati da virgola, con
--                   intervalli (es. 00-07 | 41-ff | 05,1a); vuoto = nessuno
--   AUDIO_DA        frame del primo codice (predefinito 120)
--   AUDIO_PASSO     frame fra un codice e il successivo (predefinito 90)
--   AUDIO_GIOCO     1 = gioco normale: niente suspend, moneta e start ai frame
--                   1510-1580, input casuali dal 1900 come trace.lua;
--                   altrimenti il Z80 principale è fermato con «suspend 0»
--                   al frame 100 e i codici arrivano solo da questo script
--   AUDIO_TRACE_DA  frame di apertura della traccia del solo :audiocpu in
--   AUDIO_TRACE_A   out/audio/<nome>_trace.txt e di chiusura (vuoti = nessuna traccia)
--   AUDIO_FINE      frame di uscita (predefinito: ultimo codice + passo)
--   AUDIO_FOTO      1 = foto della RAM 4000-4243 a fine frame, nel frame
--                   prima di ogni codice forzato e in quello del codice, in
--                   out/audio/<nome>_foto.txt: «frame 4000 byte» (580 byte
--                   in esadecimale maiuscolo); altrimenti nessuna foto
-- Uscite in out/audio/:
--   <nome>_ay.txt     oracolo: «frame chip reg val» solo quando un registro
--                     cambia valore (chip 1-3 = porte 10/20/30, esadecimali),
--                     più «# codice <frame> <cc>» a ogni codice forzato
--   <nome>_latch.txt  scritture a e800 dal principale: «frame valore origine»
--                     (origine gioco|forzato)
--   <nome>_ram.txt    scritture alla RAM audio 4000-47ff dal frame 110:
--                     «indirizzo scritture primo_frame ultimo_frame»
-- Stampa «CODICE <frame> <cc>» per ogni codice forzato e a fine corsa
-- «PORTE IGNOTE p=n ...» con le scritture io fuori da 10/11/20/21/30/31.
local nome = os.getenv("AUDIO_NOME") or "audio"
local function numero(v, predef)
  if v == nil or v == "" then return predef end
  return assert(tonumber(v), "numero non valido: " .. v)
end
local da = numero(os.getenv("AUDIO_DA"), 120)
local passo = numero(os.getenv("AUDIO_PASSO"), 90)
local gioco = os.getenv("AUDIO_GIOCO") == "1"
local trace_da = numero(os.getenv("AUDIO_TRACE_DA"), nil)
local trace_a = numero(os.getenv("AUDIO_TRACE_A"), nil)
local foto = os.getenv("AUDIO_FOTO") == "1"

-- elenco dei codici: «00-07,1a» -> {0,1,...,7,0x1a}
local codici = {}
for voce in string.gmatch(os.getenv("AUDIO_CODICI") or "", "[^,%s]+") do
  local a, b = string.match(voce, "^(%x+)-(%x+)$")
  if a then
    for c = tonumber(a, 16), tonumber(b, 16) do codici[#codici + 1] = c end
  else
    codici[#codici + 1] = assert(tonumber(voce, 16), "codice non valido: " .. voce)
  end
end
local fine = numero(os.getenv("AUDIO_FINE"), da + #codici * passo)
-- frame -> codice forzato
local piano = {}
for i, c in ipairs(codici) do piano[da + (i - 1) * passo] = c end

local file_traccia = string.format("out/audio/%s_trace.txt", nome)
os.execute("mkdir -p out/audio")
local f_ay = assert(io.open(string.format("out/audio/%s_ay.txt", nome), "w"))
f_ay:write("# frame\tchip\treg\tval\n")
local f_latch = assert(io.open(string.format("out/audio/%s_latch.txt", nome), "w"))
f_latch:write("# frame\tvalore\torigine\n")
local f_foto = nil
if foto then
  f_foto = assert(io.open(string.format("out/audio/%s_foto.txt", nome), "w"))
  f_foto:write("# frame\tinizio\tbyte\n")
end

frames = 0
local main = manager.machine.devices[":maincpu"].spaces["program"]
local audio = manager.machine.devices[":audiocpu"]
local dbg = manager.machine.debugger

-- oracolo AY: registro selezionato e copia ombra per chip
local selezionato = { nil, nil, nil }
local ombra = { {}, {}, {} }
local ignote = {}
TAP_AY = audio.spaces["io"]:install_write_tap(0x00, 0xff, "ay_audio", function(offset, data, mask)
  local porta = offset & 0xff
  local chip = porta >> 4
  local v = data & 0xff
  if chip < 1 or chip > 3 or (porta & 0x0f) > 1 then
    ignote[porta] = (ignote[porta] or 0) + 1
    return
  end
  if porta & 1 == 0 then
    selezionato[chip] = v
  else
    local reg = selezionato[chip]
    if reg == nil then
      ignote[porta] = (ignote[porta] or 0) + 1   -- dato senza registro selezionato
      return
    end
    if ombra[chip][reg] ~= v then
      ombra[chip][reg] = v
      f_ay:write(string.format("%d\t%X\t%02X\t%02X\n", frames, chip, reg, v))
    end
  end
end)

-- scritture al latch e800 dal lato principale
local forzando = false
TAP_LATCH = main:install_write_tap(0xe800, 0xe800, "latch_audio", function(offset, data, mask)
  f_latch:write(string.format("%d\t%02X\t%s\n", frames, data & 0xff, forzando and "forzato" or "gioco"))
end)

-- scritture alla RAM audio (installato al frame 110)
local ram = {}
TAP_RAM = nil
local function installa_ram()
  TAP_RAM = audio.spaces["program"]:install_write_tap(0x4000, 0x47ff, "ram_audio", function(offset, data, mask)
    local r = ram[offset]
    if r then
      r[1] = r[1] + 1
      r[3] = frames
    else
      ram[offset] = { 1, frames, frames }
    end
  end)
end

-- ingressi per AUDIO_GIOCO=1 (come trace.lua)
local porte = manager.machine.ioport.ports
local moneta = porte[":SYSTEM"].fields["Coin 1"]
local start = porte[":SYSTEM"].fields["1 Player Start"]
local campi = {}
for i, n in ipairs({ "P1 Left", "P1 Right", "P1 Up", "P1 Down", "P1 Button 1", "P1 Button 2" }) do
  campi[i] = porte[":P1"].fields[n]
end
local premuto = nil
math.randomseed(1)

local function chiudi()
  if premuto then premuto:set_value(0) end
  local t = {}
  for p, n in pairs(ignote) do t[#t + 1] = p end
  table.sort(t)
  local s = {}
  for _, p in ipairs(t) do s[#s + 1] = string.format("%02X=%d", p, ignote[p]) end
  print("PORTE IGNOTE " .. (#s > 0 and table.concat(s, " ") or "nessuna"))
  local indirizzi = {}
  for a in pairs(ram) do indirizzi[#indirizzi + 1] = a end
  table.sort(indirizzi)
  local f = assert(io.open(string.format("out/audio/%s_ram.txt", nome), "w"))
  f:write("# indirizzo\tscritture\tprimo_frame\tultimo_frame\n")
  for _, a in ipairs(indirizzi) do
    local r = ram[a]
    f:write(string.format("%04X\t%d\t%d\t%d\n", a, r[1], r[2], r[3]))
  end
  f:close()
  f_ay:close()
  f_latch:close()
  if f_foto then f_foto:close() end
end

if trace_da == 0 then
  dbg:command(string.format("trace %s,:audiocpu,noloop", file_traccia))
  print(string.format("TRACCIA APERTA al frame 0 (%s)", nome))
end
dbg:command("go")

emu.register_frame_done(function()
  frames = frames + 1
  -- foto della RAM a fine del frame appena finito (frames - 1), prima di
  -- ogni altra azione: frame prima del codice e frame del codice
  if f_foto and (piano[frames - 1] or piano[frames]) then
    local mem = audio.spaces["program"]
    local b = {}
    for a = 0x4000, 0x4243 do b[#b + 1] = string.format("%02X", mem:read_u8(a)) end
    f_foto:write(string.format("%d\t4000\t%s\n", frames - 1, table.concat(b)))
  end
  if not gioco and frames == 100 then dbg:command("suspend 0") end
  if frames == 110 then installa_ram() end
  if gioco then
    if frames == 1510 then moneta:set_value(1) end
    if frames == 1530 then moneta:set_value(0) end
    if frames == 1560 then start:set_value(1) end
    if frames == 1580 then start:set_value(0) end
    if frames >= 1900 and frames < fine and (frames - 1900) % 8 == 0 then
      if premuto then premuto:set_value(0) end
      premuto = campi[math.random(#campi)]
      premuto:set_value(1)
    end
  end
  if trace_da and frames == trace_da and frames > 0 then
    dbg:command(string.format("trace %s,:audiocpu,noloop", file_traccia))
    print(string.format("TRACCIA APERTA al frame %d (%s)", frames, nome))
  end
  local c = piano[frames]
  if c then
    f_ay:write(string.format("# codice %d %02X\n", frames, c))
    print(string.format("CODICE %d %02X", frames, c))
    forzando = true
    main:write_u8(0xe800, c)
    forzando = false
  end
  if trace_a and frames == trace_a then
    dbg:command("trace off")
    print(string.format("TRACCIA CHIUSA al frame %d (%s)", frames, nome))
  end
  if frames == fine then
    chiudi()
    manager.machine:exit()
  end
end)
