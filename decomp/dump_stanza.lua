-- Dump c000-ffff di una stanza forzata, come i ram_room_N.bin del brief 01.
-- Uso (da decomp/): ROOM=0 DUMP_FRAME=2000 mame solomon ... -video none -autoboot_script dump_stanza.lua
-- ROOM: stanza 0-based (tap in lettura su c243 fra i frame 1500 e 1950),
-- predefinita 0. DUMP_FRAME: frame del dump (predefinito 2000), anche più
-- frame separati da virgole (es. 1700,2000). Scrive out/dump_stanza_<ROOM>_<FRAME>.bin.
local stanza = tonumber(os.getenv("ROOM") or "0")
local voluti = {}
local ultimo = 0
for t in string.gmatch(os.getenv("DUMP_FRAME") or "2000", "[^,]+") do
  local n = tonumber(t)
  voluti[n] = true
  if n > ultimo then ultimo = n end
end
local frames = 0
local mem = manager.machine.devices[":maincpu"].spaces["program"]
local porte = manager.machine.ioport.ports
local moneta = porte[":SYSTEM"].fields["Coin 1"]
local start = porte[":SYSTEM"].fields["1 Player Start"]

local function scarica(nome)
  local f = io.open(nome, "wb")
  local t = {}
  for a = 0xc000, 0xffff do t[#t + 1] = string.char(mem:read_u8(a)) end
  f:write(table.concat(t))
  f:close()
  print("DUMP", nome, frames)
end

-- handle globale: con una local il tap sparisce col garbage collector
TAP_STANZA = nil
emu.register_frame_done(function()
  frames = frames + 1
  if frames == 1500 then
    TAP_STANZA = mem:install_read_tap(0xc243, 0xc243, "stanza", function(o, d, m) return stanza end)
  end
  if frames == 1950 and TAP_STANZA then TAP_STANZA:remove() TAP_STANZA = nil end
  if frames == 1510 then moneta:set_value(1) end
  if frames == 1530 then moneta:set_value(0) end
  if frames == 1560 then start:set_value(1) end
  if frames == 1580 then start:set_value(0) end
  if voluti[frames] then
    scarica(string.format("out/dump_stanza_%d_%d.bin", stanza, frames))
  end
  if frames >= ultimo then manager.machine:exit() end
end)
