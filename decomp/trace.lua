-- Traccia completa (noloop) del Z80 principale con il debugger di MAME.
-- Uso (da decomp/): mame solomon ... -debug -debugger none -autoboot_script trace.lua
-- TRACE_FRAMES = frame finale (predefinito 5500). Con 1900 nessun input casuale.
local fine = tonumber(os.getenv("TRACE_FRAMES") or "5500")
local frames = 0
local dbg = manager.machine.debugger
local porte = manager.machine.ioport.ports
local moneta = porte[":SYSTEM"].fields["Coin 1"]
local start = porte[":SYSTEM"].fields["1 Player Start"]
local nomi = { "P1 Left", "P1 Right", "P1 Up", "P1 Down", "P1 Button 1", "P1 Button 2" }
local campi = {}
for i, n in ipairs(nomi) do campi[i] = porte[":P1"].fields[n] end
local premuto = nil
math.randomseed(1)

dbg:command("trace out/trace.txt,0,noloop")
dbg:command("go")

emu.register_frame_done(function()
  frames = frames + 1
  -- stessi frame di roms/decoded/mame/dump2.lua per moneta e start
  if frames == 1510 then moneta:set_value(1) end
  if frames == 1530 then moneta:set_value(0) end
  if frames == 1560 then start:set_value(1) end
  if frames == 1580 then start:set_value(0) end
  -- input casuali: ogni 8 frame un campo diverso tenuto premuto 8 frame
  if frames >= 1900 and frames < fine and (frames - 1900) % 8 == 0 then
    if premuto then premuto:set_value(0) end
    premuto = campi[math.random(#campi)]
    premuto:set_value(1)
  end
  if frames == fine then
    if premuto then premuto:set_value(0) end
    dbg:command("trace off")
    print("TRACCIA CHIUSA al frame", frames)
    manager.machine:exit()
  end
end)
