-- Registro dello spedidore 3AF3 e delle scritture nella tabella attività c000-c03f.
-- Uso (da decomp/): mame solomon ... -video none -autoboot_script sched_log.lua
-- SCHED_FRAMES = frame finale (predefinito 5500). Input come trace.lua.
-- Scrive out/sched_log.txt, nell'ordine in cui avvengono:
--   D <frame> <slot> <codice>       spedizione: lettura a 3AFA con valore != 0
--   W <frame> <pc> <slot> <valore> <letture>  scrittura nella tabella; letture =
--                                   letture a 3AFA dopo l'ultima D: 0 = durante il
--                                   corpo dell'attività, m > 0 = la NMI (posta 0267,
--                                   moneta) è entrata dopo la m-esima lettura della
--                                   scansione successiva
--   I <frame> <pc> <indirizzo> <valore>  scrittura di c04e/c04f fuori da 0493
--                                   (049B) e dallo spedidore (3B0F): init e test RAM
-- frame = frame completati prima dell'evento (decimale); pc = PC dopo
-- l'istruzione di scrittura (04AA ld (hl),d -> 04AB); slot, codice, valore esadecimali.
local fine = tonumber(os.getenv("SCHED_FRAMES") or "5500")
local frames = 0
local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local porte = manager.machine.ioport.ports
local moneta = porte[":SYSTEM"].fields["Coin 1"]
local start = porte[":SYSTEM"].fields["1 Player Start"]
local nomi = { "P1 Left", "P1 Right", "P1 Up", "P1 Down", "P1 Button 1", "P1 Button 2" }
local campi = {}
for i, n in ipairs(nomi) do campi[i] = porte[":P1"].fields[n] end
local premuto = nil
math.randomseed(1)

local righe = {}
local function svuota(f)
  f:write(table.concat(righe))
  righe = {}
end
local uscita = io.open("out/sched_log.txt", "w")
local letture = 0     -- letture a 3AFA dopo l'ultima spedizione

-- handle globali: altrimenti il garbage collector toglie i tap
TAP_LETTURA = mem:install_read_tap(0xc000, 0xc03f, "sched_lettura", function(offset, data, mask)
  if cpu.state["PC"].value ~= 0x3AFB then return end
  local v = data & 0xff
  if v ~= 0 then
    righe[#righe + 1] = string.format("D %d %02X %02X\n", frames, offset & 0x3f, v)
    letture = 0
  else
    letture = letture + 1
  end
end)
TAP_SCRITTURA = mem:install_write_tap(0xc000, 0xc03f, "sched_scrittura", function(offset, data, mask)
  righe[#righe + 1] = string.format("W %d %04X %02X %02X %d\n", frames,
    cpu.state["PC"].value, offset & 0x3f, data & 0xff, letture)
end)
TAP_INDICI = mem:install_write_tap(0xc04e, 0xc04f, "sched_indici", function(offset, data, mask)
  local pc = cpu.state["PC"].value
  if pc ~= 0x049B and pc ~= 0x3B0F then
    righe[#righe + 1] = string.format("I %d %04X %04X %02X\n", frames, pc, offset, data & 0xff)
  end
end)

emu.register_frame_done(function()
  frames = frames + 1
  if #righe > 20000 then svuota(uscita) end
  if frames == 1510 then moneta:set_value(1) end
  if frames == 1530 then moneta:set_value(0) end
  if frames == 1560 then start:set_value(1) end
  if frames == 1580 then start:set_value(0) end
  if frames >= 1900 and frames < fine and (frames - 1900) % 8 == 0 then
    if premuto then premuto:set_value(0) end
    premuto = campi[math.random(#campi)]
    premuto:set_value(1)
  end
  if frames == fine then
    if premuto then premuto:set_value(0) end
    TAP_LETTURA:remove()
    TAP_SCRITTURA:remove()
    TAP_INDICI:remove()
    svuota(uscita)
    uscita:close()
    print("SCHED: registro chiuso al frame", frames)
    manager.machine:exit()
  end
end)
