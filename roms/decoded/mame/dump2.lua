local frames = 0
local mem = manager.machine.devices[":maincpu"].spaces["program"]
local ports = manager.machine.ioport.ports
local coin, start
for pname, port in pairs(ports) do
  for fname, field in pairs(port.fields) do
    print("PORT", pname, fname)
    if fname == "Coin 1" then coin = field end
    if fname == "1 Player Start" then start = field end
  end
end
local function dump(name)
  local f = io.open(name, "wb")
  for a = 0xc000, 0xffff do f:write(string.char(mem:read_u8(a))) end
  f:close()
  manager.machine.video:snapshot()
  print("DUMPED", name, frames)
end
emu.register_frame_done(function()
  frames = frames + 1
  if frames == 1500 then dump("ram_title.bin") end
  if frames == 1510 then coin:set_value(1) end
  if frames == 1530 then coin:set_value(0) end
  if frames == 1560 then start:set_value(1) end
  if frames == 1580 then start:set_value(0) end
  if frames == 1900 then dump("ram_level1.bin") end
  if frames == 1920 then manager.machine:exit() end
end)
