# Stato del progetto

Aggiornato al 2026-11-06. Fotografia operativa per riprendere il lavoro:
qui c'è solo lo stato attuale; il metodo è nel [README](README.md), la
storia di ogni ciclo nei brief in `compiti/` (l'esito in coda a ciascuno).

## Fotografia

Il gioco è **completo, giocabile e autosufficiente**: tutto il Z80
principale e tutto il driver audio sono tradotti in C, lo strato hardware
SDL2 è finito, e dal lotto 63 il binario `gioca` **non legge più nessuna
ROM** — i dati Tecmo (91 tabelle del programma, grafica, musica, driver
audio) vivono come sorgente C generato in `tecmo/`, con estrattori e
round-trip byte per byte. Un clone pulito del repo compila e gioca.

Release pubblicate:

- **v1.0-rc1** (2026-11-05): primo binario giocabile, richiedeva le ROM.
- **v1.01** (2026-11-06): «click and play», eseguibile Linux x86_64 negli
  asset, nessuna ROM richiesta.
  <https://github.com/rufio72/solomonKey/releases/tag/v1.01>
  Binario sostituito la sera stessa dopo i lotti 65-66B (bug dei mostri
  dal 3-3 in poi e stop del corpo 07 risolti); note della release aggiornate.

## Numeri

| Cosa | Valore |
|---|---|
| Motore dei nemici | completo, 220/220 routine |
| Codici della tabella corpi 3B11 | 40/40 tradotti |
| Banco di prova (`harness --tutte`) | 335 voci, 1439 catture, 0 fallite |
| Tabelle Tecmo in `tecmo/` (brief 62-66B) | 93 voci del programma (21849 byte) + driver audio + musica + grafica |
| Letture ROM a runtime | 0 (`strace`: né `main64.bin` né `1.3jk`) |
| Giro completo | 0-5499, saldatura 0 byte, 0 divergenze nello spedidore, 13 frame con hash coincidenti |
| Destinazioni mai osservate | 59 in `decomp/ZONE-MAI-ESEGUITE.md` (53 DATO, 6 RAGGIUNGIBILI) |

## Fasi chiuse (dettagli nei brief indicati)

- **Analisi e traduzione del Z80 principale** (brief 01-16, 24-40):
  scheduler, NMI, motore oggetti, porte, motore dei nemici N1-N12,
  bonus/finale, chiave 0D18, zone mai eseguite. Finita il 2026-11-03.
- **Z80 audio** (brief 18-22): driver tradotto per intero, dati estratti,
  wav dal C identici a MAME in 40/44 (le 4 differenze giudicate OK
  all'ascolto). Catena audio completa.
- **Replay e strato hardware H1-H7** (brief 23, 41-51): oracolo video,
  renderizzatore pixel-perfetto, finestra SDL2, giro dal boot saldato
  all'oracolo, input (tastiera, DSW, joystick DAIJA con hotplug,
  registra/riproduci), audio dal vivo. Il C fa il boot dal reset e gioca.
- **Gioco libero completo** (brief 52-59): tutti i corpi, raccolta con
  effetto, registro R surrogato (LCG), cadenza del motore dei nemici a
  costo (`MACCHINA_COSTO`), stanze bonus/finale, classifica, morte e
  riavvio attract senza FERMO; `--scanlines`. Lotto 65 (brief
  `compiti/65-routine-39b9.md`): fase 0 del corpo 07 completa (3874 passaggio
  di quadro, 38BA, 38CA, 38D9, 398B, 3986, 39B9 «DEMO») e 365F/366C della
  fase 4; il libero senza ingressi corre 30000 frame senza FERMO (hash
  cfde832053b94095 / audio 58cc653154347157), semi 4 e 67 40000 frame senza
  FERMO; `copione_demo_767c` allargato a 767C-78DF.
- **Mappe degli oggetti nascosti** (brief 60): `docs/special/` con le 52
  stanze, 128 oggetti in 40 stanze, renderizzatore Python identico al C.
- **Autosufficienza** (brief 61-63): censimento delle letture ROM
  (`decomp/CENSIMENTO-ROM.md` + `censimento_rom.py --verifica`), cartella
  `tecmo/` con tutte le tabelle, blob del driver audio, `make gioco`
  senza ROM. Sentinella «lettura fuori voce» sempre attiva (legge 0 e
  avvisa su stderr).
- **Pubblicazione** (brief 64): README e NOTICE in inglese, pacchetto e
  release v1.01.

## Lavoro aperto, in ordine

1. **Collaudo dell'utente col binario autosufficiente**: partita vera
   guardando stderr — un avviso della sentinella «fuori voce» significa
   una tabella del censimento ancora corta, da allargare con
   `estrai_rom.py` (le estensioni «misurate/stimate» sono annotate in
   `CENSIMENTO-ROM.md`).
2. **Lotto 67 (in coda, deciso)**: versione browser — C+SDL2 compilato in
   WebAssembly con Emscripten, pubblicata su GitHub Pages, link nel
   README. Niente filesystem virtuale: i dati sono già C compilato.
3. **Code minori**: 39B9 chiusa nel lotto 65; 2E04 (fase 11 del 07),
   2D0F (fase 5, stanza 29) e le sette zone «fuori voce» chiusi nel lotto
   66: la spazzata delle 52 stanze (`--stanza N -n 3000 --riproduci` seme
   4) non dà né FERMO né AVVISO. Nel lotto 66B chiusa anche 2D9D (fase 16
   del 07, passi del corpo 18 a 416B) e 3065: nessun `nemici_manca` resta
   nei file del corpo 07; spazzata a 8000 frame senza FERMO né AVVISO
   (stanza 5: fe80c34bdbf49e09 / dcf24f542f52de6f); fondale
   322A avanti ~5 frame nel libero (cadenza, vedi sotto); mappa dei
   pulsanti nel README (quale è BUTTON1/BUTTON2; mappa DAIJA predefinita
   0,1,7,6); classi special 0x16/0x17/0x35 mai viste in chiaro (si
   verificano solo se una mappa mostra un oggetto sospetto).
4. **Fase finale: riscrittura leggibile** (piano ratificato dall'utente):
   refactoring modulo per modulo nello stesso repo, un branch corto per
   modulo, banco+giro+replay come regressione, la versione fedele resta
   come riferimento.

**Tema da tenere d'occhio — cadenza del gioco libero** (decisione utente
2026-11-04: «ci ritorniamo»). Il modo libero chiude il frame con
un'euristica (~22 corpi/frame) più il costo dichiarato dal motore dei
nemici; non è il costo vero delle istruzioni. Ogni difetto visivo o di
ritmo nel libero va sospettato prima come cadenza, poi come traduzione.
Caso noto e misurato: il fondale 322A (in MAME ~6 frame con NMI interne,
nel libero uno solo; vedi `decomp/GIRO.md`).

## Regressioni (devono restare verdi dopo ogni modifica)

- `sh decomp/giro_completo.sh` → «GIRO COMPLETO 0-5499: saldatura 0 byte,
  spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con
  hash coincidenti».
- Le ricette in `decomp/ricette/` con gli hash annotati in `decomp/GIRO.md`.
- Modo `--oracolo` (hash video `a239d0a940bec0b9`, audio `c4b40fea06623a96`).
- Banco: `harness --tutte` → 1439 catture, 1439 OK.
- `python3 decomp/censimento_rom.py --verifica` → «CENSIMENTO OK: 202
  occorrenze, 0 non classificate» e «LETTI FUORI VOCE: 0».
- Spazzata delle 52 stanze: `gioca --stanza N --riproduci` (seme 4) a 8000
  frame, per N da 0 a 51 — zero FERMO, zero AVVISO.
- Clone pulito senza `roms/`: `make -C decomp/harness gioco` compila e
  `gioca -n 600 --muto` dà `ecd5acf694285489` / `8aedf41c0424df54`.

## Come si lavora (ricetta collaudata)

- Ciclo per lotto, tutto in locale: brief dell'analista (fatti verificati,
  verifiche a uscita attesa) → sviluppatore → verificatore → esito
  annotato in coda al brief e commit. Niente push salvo richiesta.
- Schema economico quando il lotto è di routine: analista e sviluppatore
  Sonnet, verifica a script; modelli più alti solo sui lotti di confini
  (griglia nei brief). Le sessioni cloud sono dismesse dal 2026-10-29.
- Le ROM servono solo agli strumenti di sviluppo (`decomp/build_rom.sh`,
  estrattori, catture MAME): `solomon.zip` in `roms/`, mai committata.
- MAME senza finestra: `-video none -sound none` più
  `SDL_VIDEODRIVER=dummy` (già negli script); mai pkill larghi su
  `gioca`/`mame`.
