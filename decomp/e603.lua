-- Protezione e603: valore letto dal Z80 principale per ogni PC di lettura.
-- Uso (da decomp/): mame solomon ... -video none -autoboot_script e603.lua
-- Variabili d'ambiente: E603_FRAMES (frame finale, predefinito 5500),
-- E603_ROOM (facoltativo, stanza 0-based forzata con un tap su c243 fra i
-- frame 1500 e 1950). Moneta, start e input casuali come trace.lua.
-- Scrive out/e603.txt (righe «PC valore conteggio», PC del tap = PC dopo
-- l'istruzione) e harness/e603.h (PC riportato all'istruzione: tap - 3).
local fine = tonumber(os.getenv("E603_FRAMES") or "5500")
local stanza = os.getenv("E603_ROOM")
if stanza then stanza = tonumber(stanza) end
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

-- conteggi[PC][valore]; gli handle dei tap restano in variabili globali,
-- altrimenti il garbage collector li rimuove dopo poche decine di frame
local conteggi = {}
TAP_E603 = mem:install_read_tap(0xe603, 0xe603, "e603", function(offset, data, mask)
  local pc = cpu.state["PC"].value
  local per_pc = conteggi[pc]
  if not per_pc then per_pc = {} conteggi[pc] = per_pc end
  per_pc[data & 0xff] = (per_pc[data & 0xff] or 0) + 1
end)
TAP_STANZA = nil

local function scrivi()
  local chiavi = {}
  for pc, per_pc in pairs(conteggi) do
    for v, n in pairs(per_pc) do chiavi[#chiavi + 1] = { pc, v, n } end
  end
  table.sort(chiavi, function(x, y) return x[1] < y[1] or (x[1] == y[1] and x[2] < y[2]) end)
  local f = io.open("out/e603.txt", "w")
  for _, k in ipairs(chiavi) do f:write(string.format("%04X %02X %d\n", k[1], k[2], k[3])) end
  f:close()
  local h = io.open("harness/e603.h", "w")
  h:write("/* Generato da e603.lua: non modificare a mano.\n")
  h:write(" * Protezione e603: valore restituito da MAME per PC dell'istruzione di lettura. */\n")
  h:write("#ifndef E603_H\n#define E603_H\n\n#include <stdint.h>\n\n")
  h:write("static const struct { uint16_t pc_lettura; uint8_t valore; } e603_tabella[] = {\n")
  for _, k in ipairs(chiavi) do
    h:write(string.format("    {0x%04X, 0x%02X},  /* %d letture */\n", (k[1] - 3) & 0xffff, k[2], k[3]))
  end
  h:write("};\n\n#endif\n")
  h:close()
end

emu.register_frame_done(function()
  frames = frames + 1
  if stanza and frames == 1500 then
    TAP_STANZA = mem:install_read_tap(0xc243, 0xc243, "stanza", function(o, d, m) return stanza end)
  end
  if stanza and frames == 1950 and TAP_STANZA then TAP_STANZA:remove() TAP_STANZA = nil end
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
    scrivi()
    print("E603: tabella scritta al frame", frames)
    manager.machine:exit()
  end
end)
