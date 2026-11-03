-- Oracolo video (lotto H1): dump di video RAM, sprite RAM e palette RAM più lo
-- screenshot PNG di MAME presi nella stessa callback frame_done.
-- Variabili d'ambiente: VIDEO_STATO (nome dei file), VIDEO_DIR (cartella delle
-- uscite), VIDEO_FRAME (frame del dump), VIDEO_INGRESSI (nessuno|traccia),
-- VIDEO_ROOM (stanza forzata 0-based, vuota = nessuna), VIDEO_POKE e
-- VIDEO_POSTA (formati «frame:addr:val» e «frame:codice», separati da virgola,
-- 0x ammesso). Inneschi e ingressi sono copiati da snap.lua.
local stato = os.getenv("VIDEO_STATO") or "titolo"
local dir = os.getenv("VIDEO_DIR") or "out/video"
local frame_dump = tonumber(os.getenv("VIDEO_FRAME") or "1000")
local ingressi = os.getenv("VIDEO_INGRESSI") or "nessuno"
local function inneschi(testo)
  local t = {}
  for voce in string.gmatch(testo or "", "[^,%s]+") do
    local v = {}
    for x in string.gmatch(voce, "[^:]+") do v[#v + 1] = assert(tonumber(x), "numero non valido: " .. x) end
    t[#t + 1] = v
  end
  return t
end
local stanza = os.getenv("VIDEO_ROOM") or ""
if stanza == "" then stanza = nil else stanza = assert(tonumber(stanza), "VIDEO_ROOM non valida") end
local pokes = inneschi(os.getenv("VIDEO_POKE"))
local poste = inneschi(os.getenv("VIDEO_POSTA"))

local mem = manager.machine.devices[":maincpu"].spaces["program"]
local frames = 0
local TAP_STANZA
local FINE_TAP = 1950

-- Ingressi «traccia»: stessa sequenza di snap.lua.
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
    if p[1] == frames then mem:write_u8(p[2], p[3]) end
  end
  for _, p in ipairs(poste) do
    if p[1] == frames then
      for a = 0xc000, 0xc03f do
        if mem:read_u8(a) == 0 then mem:write_u8(a, p[2]) break end
      end
    end
  end
end

local function scarica(nome, da, a)
  local t = {}
  for i = da, a do t[#t + 1] = string.char(mem:read_u8(i)) end
  local f = assert(io.open(string.format("%s/%s_%s.bin", dir, stato, nome), "wb"))
  f:write(table.concat(t))
  f:close()
end

emu.register_frame_done(function()
  frames = frames + 1
  if stanza and frames == 1500 then
    TAP_STANZA = mem:install_read_tap(0xc243, 0xc243, "stanza_video", function(o, d, m) return stanza end)
  end
  if stanza and frames == FINE_TAP and TAP_STANZA then TAP_STANZA:remove() TAP_STANZA = nil end
  muovi_ingressi()
  applica_inneschi()
  if frames == frame_dump then
    -- dump e PNG nella stessa callback, senza far girare la CPU in mezzo
    scarica("bg_colorram", 0xd800, 0xdbff)
    scarica("bg_videoram", 0xdc00, 0xdfff)
    scarica("fg_colorram", 0xd000, 0xd3ff)
    scarica("fg_videoram", 0xd400, 0xd7ff)
    scarica("spriteram", 0xe000, 0xe07f)
    scarica("palette", 0xe400, 0xe5ff)
    manager.machine.video:snapshot()
    print(string.format("VIDEO: stato %s frame %d", stato, frames))
    manager.machine:exit()
  end
end)
