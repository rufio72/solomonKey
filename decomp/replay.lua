-- Oracolo del replay (brief 23): impronta della RAM a ogni NMI e registro
-- dello spedidore, con gli ingressi convenzionali (come sched_log.lua).
-- Uso (da decomp/): sh replay.sh, che lancia MAME con -debug e questo script.
-- Variabili d'ambiente:
--   REPLAY_FINE  frame finale (predefinito 5500)
--   REPLAY_F0    frame di partenza del replay: dump completo (predefinito 2402)
--   REPLAY_DUMPS altri frame con dump completo, separati da virgola (vuota)
--   REPLAY_NOME  prefisso dei file in out/ (predefinito replay)
-- Prodotti:
--   out/<nome>_oracolo.txt  una riga per NMI:
--       F <f> <e600> <e601> <e602> <ritorno> <hash>
--     f = frame emulati (numero di frame dello schermo + 1) allo stop su 0066;
--     e600-e602 = valori vivi delle porte; ritorno = indirizzo di ritorno
--     della NMI letto in cima alla pila (dice dove è entrata: spedidore
--     3AF3-3B10 o a metà attività); hash = FNV-1a 32 bit di c000-e5ff
--     esclusi c040-c043 (IX/IY salvati) e cf00-cfff (pila).
--   out/<nome>_disp.txt  righe D/W/I identiche a sched_log.lua, più una
--     riga «N <f> <ritorno> <letture>» nel punto in cui entra la NMI del
--     frame f (aggiunta al formato: dice fra quali spedizioni cade la NMI;
--     letture = letture a 3AFA dopo l'ultima D, come il campo delle W).
--   out/<nome>_<f>.bin  c000-ffff (16384 byte) a F0 e ai frame di REPLAY_DUMPS.
local fine = tonumber(os.getenv("REPLAY_FINE") or "5500")
local f0 = tonumber(os.getenv("REPLAY_F0") or "2402")
local nome = os.getenv("REPLAY_NOME") or "replay"
local dumps = {}
dumps[f0] = true
for x in string.gmatch(os.getenv("REPLAY_DUMPS") or "", "[^,%s]+") do
  dumps[assert(tonumber(x), "frame non valido: " .. x)] = true
end

local frames = 0
local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local dbg = manager.machine.debugger
local porte = manager.machine.ioport.ports
local moneta = porte[":SYSTEM"].fields["Coin 1"]
local start = porte[":SYSTEM"].fields["1 Player Start"]
local nomi = { "P1 Left", "P1 Right", "P1 Up", "P1 Down", "P1 Button 1", "P1 Button 2" }
local campi = {}
for i, n in ipairs(nomi) do campi[i] = porte[":P1"].fields[n] end
local premuto = nil
math.randomseed(1)

local righe = {}
local oracolo = {}
local uscita = io.open("out/" .. nome .. "_disp.txt", "w")
local f_oracolo = io.open("out/" .. nome .. "_oracolo.txt", "w")
local function svuota()
  uscita:write(table.concat(righe))
  righe = {}
  f_oracolo:write(table.concat(oracolo))
  oracolo = {}
end
local letture = 0     -- letture a 3AFA dopo l'ultima spedizione
local chiuso = false

-- Tap identici a sched_log.lua (handle globali contro il garbage collector).
TAP_LETTURA = mem:install_read_tap(0xc000, 0xc03f, "replay_lettura", function(offset, data, mask)
  if cpu.state["PC"].value ~= 0x3AFB then return end
  local v = data & 0xff
  if v ~= 0 then
    righe[#righe + 1] = string.format("D %d %02X %02X\n", frames, offset & 0x3f, v)
    letture = 0
  else
    letture = letture + 1
  end
end)
TAP_SCRITTURA = mem:install_write_tap(0xc000, 0xc03f, "replay_scrittura", function(offset, data, mask)
  righe[#righe + 1] = string.format("W %d %04X %02X %02X %d\n", frames,
    cpu.state["PC"].value, offset & 0x3f, data & 0xff, letture)
end)
TAP_INDICI = mem:install_write_tap(0xc04e, 0xc04f, "replay_indici", function(offset, data, mask)
  local pc = cpu.state["PC"].value
  if pc ~= 0x049B and pc ~= 0x3B0F then
    righe[#righe + 1] = string.format("I %d %04X %04X %02X\n", frames, pc, offset, data & 0xff)
  end
end)

-- FNV-1a a 32 bit su un blocco letto come stringa.
local function fnv(h, s)
  for i = 1, #s do
    h = ((h ~ string.byte(s, i)) * 16777619) & 0xffffffff
  end
  return h
end

-- Impronta di c000-e5ff esclusi c040-c043 e cf00-cfff (stesse zone di replay.c).
local function impronta()
  local h = 2166136261
  h = fnv(h, mem:read_range(0xc000, 0xc03f, 8))
  h = fnv(h, mem:read_range(0xc044, 0xceff, 8))
  h = fnv(h, mem:read_range(0xd000, 0xe5ff, 8))
  return h
end

local function scarica(nomefile)
  local f = io.open(nomefile, "wb")
  local t = {}
  for a = 0xc000, 0xffff do t[#t + 1] = string.char(mem:read_u8(a)) end
  f:write(table.concat(t))
  f:close()
end

local function chiudi()
  if chiuso then return end
  chiuso = true
  if premuto then premuto:set_value(0) end
  TAP_LETTURA:remove()
  TAP_SCRITTURA:remove()
  TAP_INDICI:remove()
  dbg:command("bpclear")
  svuota()
  uscita:close()
  f_oracolo:close()
  print("REPLAY: oracolo chiuso al frame", frames)
  manager.machine:exit()
end

-- Ingressi convenzionali, come sched_log.lua, al frame «frames».
local function muovi_ingressi()
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

-- Il contatore dei frame viene dal numero di frame dello schermo, non dal
-- numero di chiamate a frame_done: a debugger fermo MAME continua a
-- ridisegnare e chiama frame_done anche senza emulare, un numero di volte
-- che dipende dal tempo reale. Senza debugger la prima chiamata vede il
-- frame 0 e frames = numero di frame + 1, come il contatore di sched_log.lua.
-- Misura: il registro D/W/I coincide con out/sched_log.txt riga per riga
-- a meno del campo frame:
-- con -debug lo stesso evento cade un frame dopo dal reset del ~600 in
-- poi (sched_log 601 = qui 602): F0 = 2402 qui è il 2401 di sched_log.
local schermo = manager.machine.screens[":screen"]
local base = nil
emu.register_frame_done(function()
  local n = schermo:frame_number()
  if base == nil then base = n - 1 end
  while frames < n - base and not chiuso do
    frames = frames + 1
    if #righe > 20000 then svuota() end
    muovi_ingressi()
    if frames == fine then chiudi() end
  end
end)

dbg:command("bpset 66")
dbg:command("go")

emu.register_periodic(function()
  if chiuso or dbg.execution_state ~= "stop" then return end
  if cpu.state["PC"].value ~= 0x0066 then dbg:command("go") return end
  local ritorno = mem:read_u16(cpu.state["SP"].value)
  righe[#righe + 1] = string.format("N %d %04X %d\n", frames, ritorno, letture)
  oracolo[#oracolo + 1] = string.format("F %d %02X %02X %02X %04X %08X\n", frames,
    mem:read_u8(0xe600), mem:read_u8(0xe601), mem:read_u8(0xe602), ritorno, impronta())
  if dumps[frames] then scarica(string.format("out/%s_%d.bin", nome, frames)) end
  dbg:command("go")
end)
