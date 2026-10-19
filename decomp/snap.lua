-- Stato prima/dopo di una routine del Z80 principale.
-- Variabili d'ambiente: SNAP_ENTRY (es. 0x05B3), SNAP_N (occorrenza, predefinito 1),
-- SNAP_DIR (predefinito out), SNAP_DA_FRAME (frame da cui armare il breakpoint,
-- predefinito 0: le occorrenze si contano da lì). Scrive <dir>/snap_<ENTRY>_prima.bin, _dopo.bin
-- (16384 byte c000-ffff) e _regs.txt. Le catture interrotte dalla NMI si scartano.
local testo_entry = os.getenv("SNAP_ENTRY") or "0x05B3"
local entry = tonumber(testo_entry)
local voluta = tonumber(os.getenv("SNAP_N") or "1")
local dir = os.getenv("SNAP_DIR") or "out"
local da_frame = tonumber(os.getenv("SNAP_DA_FRAME") or "0")
local base = string.format("%s/snap_%s", dir, testo_entry)

local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local dbg = manager.machine.debugger
local REG = { "PC", "AF", "BC", "DE", "HL", "IX", "IY", "SP" }

local fase = "attesa"      -- attesa | dopo | fatto
local occorrenze = 0
local scartate = 0
local riga_prima, ritorno, sp_ritorno
local frames = 0

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
  dbg:command(string.format("bpset %X", entry))
end

if da_frame == 0 then arma_ingresso() end
dbg:command("go")

emu.register_frame_done(function()
  frames = frames + 1
  if da_frame > 0 and frames == da_frame then arma_ingresso() end
  if frames >= 6000 and fase ~= "fatto" then
    print("SNAP: limite di 6000 frame raggiunto senza cattura")
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
    print(string.format("SNAP: catturata occorrenza %d di %s al frame %d, scartate %d",
      occorrenze, testo_entry, frames, scartate))
    fase = "fatto"
    dbg:command("bpclear")
    manager.machine:exit()
  end
end)
