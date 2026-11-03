-- Oracolo audio della partita del giro dal boot (lotto H6, brief 50).
-- Stessa partita di giro_boot.lua (stessi ingressi, stessa numerazione dei
-- frame) ma registra solo l'audio:
--   out/audio/partita_latch.txt  "# frame\tvalore\torigine", una riga per
--       scrittura del principale a e800 (frame = contatore al momento);
--   out/audio/partita_ay.txt     "# frame\tchip\treg\tval", solo ai cambi dei
--       registri YM2149 (stessa logica di audio.lua).
-- Variabile: REPLAY_FINE (predefinito 5500). Lanciato da partita_audio.sh.
local fine = tonumber(os.getenv("REPLAY_FINE") or "5500")
local frames = 0
local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local audio = manager.machine.devices[":audiocpu"]
local dbg = manager.machine.debugger
local porte = manager.machine.ioport.ports
local moneta = porte[":SYSTEM"].fields["Coin 1"]
local start = porte[":SYSTEM"].fields["1 Player Start"]
local campi = {}
for i, n in ipairs({ "P1 Left", "P1 Right", "P1 Up", "P1 Down", "P1 Button 1", "P1 Button 2" }) do
  campi[i] = porte[":P1"].fields[n]
end
local premuto = nil
math.randomseed(1)

local f_l = assert(io.open("out/audio/partita_latch.txt", "w"))
local f_ay = assert(io.open("out/audio/partita_ay.txt", "w"))
f_l:write("# frame\tvalore\torigine\n")
f_ay:write("# frame\tchip\treg\tval\n")
local n_latch, n_ay = 0, 0

-- registro selezionato e copia ombra per chip (come audio.lua)
local sel = {}
local ombra = { {}, {}, {} }
TAP_AY = audio.spaces["io"]:install_write_tap(0x00, 0xff, "ay", function(offset, data, mask)
  local porta = offset & 0xff
  local chip = porta >> 4
  local v = data & 0xff
  if chip < 1 or chip > 3 or (porta & 0x0f) > 1 then return end
  if porta & 1 == 0 then
    sel[chip] = v
  else
    local reg = sel[chip]
    if reg == nil then return end
    if ombra[chip][reg] ~= v then
      ombra[chip][reg] = v
      n_ay = n_ay + 1
      f_ay:write(string.format("%d\t%X\t%02X\t%02X\n", frames, chip, reg, v))
    end
  end
end)
TAP_L = mem:install_write_tap(0xe800, 0xe800, "latch", function(offset, data, mask)
  n_latch = n_latch + 1
  f_l:write(string.format("%d\t%02X\tgioco\n", frames, data & 0xff))
end)

local chiuso = false
local function chiudi()
  if chiuso then return end
  chiuso = true
  f_l:close()
  f_ay:close()
  print(string.format("PARTITA_AUDIO latch %d ay %d", n_latch, n_ay))
  manager.machine:exit()
end

local function muovi()
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

-- numerazione dei frame identica a giro_boot.lua
local schermo = manager.machine.screens[":screen"]
local base = nil
emu.register_frame_done(function()
  local n = schermo:frame_number()
  if base == nil then base = n - 1 end
  while frames < n - base and not chiuso do
    frames = frames + 1
    muovi()
    if frames == fine then chiudi() end
  end
end)

-- bpset 66 e ciclo periodico: servono solo a tenere la stessa numerazione
dbg:command("bpset 66")
dbg:command("go")
emu.register_periodic(function()
  if chiuso or dbg.execution_state ~= "stop" then return end
  dbg:command("go")
end)
