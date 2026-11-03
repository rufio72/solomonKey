# Compito 40: documentare le destinazioni mai eseguite del Z80 principale

Data: 2026-11-03. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere. Modello di forma: `compiti/39-lotto-chiave-0d18.md`.

## Obiettivo

Lotto di ANALISI E DOCUMENTAZIONE, non di traduzione. Delle 299
destinazioni statiche di `call` del Z80 principale, 64 non sono mai state
osservate in esecuzione; 5 sono nel frattempo state tradotte, ne restano
**59** (22 sotto 8000, 37 a 8000 o sopra). Per ognuna va stabilito e scritto
un verdetto: (a) DATO (non è codice), (b) MORTO (codice o falso codice
irraggiungibile: sito di chiamata e perché non si esegue), (c) RAGGIUNGIBILE
(codice vero raggiungibile in condizioni particolari: quali; se una
forzatura mirata di RAM con MAME lo sveglia a costo ragionevole, provarla
e scrivere la ricetta, senza tradurre). Prodotto: `decomp/ZONE-MAI-ESEGUITE.md`
(una riga di verdetto per destinazione, più note) e l'aggiornamento dei
censimenti pertinenti.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`, e solo:
  `decomp/ZONE-MAI-ESEGUITE.md` (nuovo), `decomp/CENSIMENTO-12.md` (sezione
  «Destinazioni ancora scoperte»), `decomp/ROUTINE.md` solo se serve un
  rimando (facoltativo), e questo brief non si tocca. `STATO.md` e
  `README.md` non si toccano (li aggiorna la chat principale).
- **Niente codice C, niente banco, niente script**: non si modificano né
  si aggiungono file in `decomp/harness/`, né `snap.lua`, `cattura*.sh`,
  `sessioni.sh`, `replay.*`, `sched*.c`, `copertura_*.{py,lua}`,
  `coverage.py`. Nessuna cattura nuova in `decomp/out/` (`snap_*`) e nessun
  file di `decomp/out/` già esistente va riscritto (in particolare
  `call_mai_osservate.txt`, `eseguiti_unione.txt`, `main48.dasm`,
  `main64.dasm`: sono ignorati da git, quindi se ne controlla l'md5, vedi
  verifica 6).
- Le forzature con MAME si PROVANO in una copia di lavoro fuori dal
  repository (per esempio `/tmp/z40/`, con `roms` collegata e `out/` vuota
  o propria): `SNAP_DIR` e le uscite di `copertura_call.py` (`--uscite`)
  vanno puntate lì. MAME sempre senza finestra: `-video none -sound none`,
  `SDL_VIDEODRIVER=dummy` e `-keyboardprovider none -mouseprovider none
  -joystickprovider none -lightgunprovider none` come in `cattura.sh`
  (r. 81-83). Mai MAME con finestra, mai partite a mano.
- Niente installazioni. Niente git distruttivo (`stash`, `reset --hard`,
  `filter-branch`). Un solo agente alla volta su `decomp/`.
- Commenti e documenti in italiano. Nessun file sopra le 800 righe: se
  `ZONE-MAI-ESEGUITE.md` ci arriva, si divide in due (il tetto vale per i
  file di codice, ma si rispetti anche qui come buona pratica).

## Cosa esiste già (fatti verificati)

Tutti i fatti sono misurati il 2026-11-03 da `decomp/` (partendo da
`/home/rufio72/OpenSolomonsKey-master`).

| risorsa | fatto |
|---|---|
| `decomp/out/call_mai_osservate.txt` | 65 righe: intestazione `# destinazione<TAB>prima call statica`, poi 64 righe `DEST<TAB>SITO` (esadecimale maiuscolo). Non tracciato (`.gitignore` r. 50: `decomp/out/*`) |
| `decomp/out/eseguiti_unione.txt` | PC eseguiti in almeno una sessione, uno per riga, esadecimale; non tracciato |
| `decomp/out/main48.dasm`, `main64.dasm` | disassemblato `z80dasm -a -l -g 0` (52160 righe il 64); ogni riga finisce con `;AAAA` (indirizzo). `main48.dasm` è la sorgente di `copertura_call.py`. `z80dasm` è in `/usr/bin` |
| `decomp/copertura_call.py` | `--classifica`, `--dasm`, `--limite`, `--uscite DIR`; legge `out/*_eseguiti.txt`; scrive nella cartella `--uscite` (mai usare quella predefinita `out`) |
| `decomp/CENSIMENTO-12.md` | in coda, sezione `## Destinazioni ancora scoperte (64)`: tabella `\| destinazione \| prima call statica \| sito_eseguito \|`, **tutte le 64 righe «NO»** (nessun sito eseguito) |
| `decomp/RAPPORTO.md` r. 31-36 | mappa ROM: programma 0000-BFFF + F000-FFFF; **F000-FFFF è dati**; zone mai eseguite: 80A5-BFFF (16 KB, «quasi certamente dati: livelli, testi, tabelle»), 12AB-2349, 5CFB-63A3, 4F1C-5506, 6C48-714A, 6785-6C1C, 3F93-4397 |
| `decomp/DATI.md`, `decomp/estrai_dati.py` | formato di livelli e tabelle (consultare per il verdetto DATO) |
| Partite dal vivo | `decomp/out/partita_viva_20261027.log` (265 righe) e `...27b.log` (131): contengono solo righe `DEST 4FC5` (7 in tutto) e `SITO 4F8F` (1); nessun'altra delle 63 ha scattato. Script `decomp/copertura_viva.lua` (58 righe, MAME con finestra, per l'utente: NON rilanciarlo) |
| Forzature di RAM | `snap.lua` e `cattura.sh` (sono la documentazione d'uso, solo da leggere): `SNAP_POKE=frame:addr:val,...`, `SNAP_POSTA=frame:codice,...`, `SNAP_FORZA=ix+HH:VV,iy+HH:VV,a|b|c|d|e|h|l:VV`, `SNAP_ROOM`, `SNAP_COND`. Stati: `titolo gioco tardo porta bonus finale finale2 attNN stanzaNN` |
| Banco | `decomp/harness/harness --tutte`: exit 0, 304 `^RIEPILOGO`, 1270 `^cattura`, 0 `FALLITO` (misurato) |

### Le 5 già tradotte (da togliere dall'elenco, da documentare in una tabella a parte)

Verificate con la definizione `void sub_XXXX(` nei sorgenti C di
`decomp/harness/` (grep sui 64 indirizzi: queste cinque e nessun'altra):

| destinazione | dove | nota |
|---|---|---|
| 4FC5 | `nemici_comparsa.c` r. 223 | fata, 7 scatti dal vivo (6 + 1), sito 4F8F eseguito 1 volta; CENSIMENTO-NEMICI r. 141 e 357 |
| 5F82 | `nemici_tipo07a.c` | chiamata da `scegli_5f4b_5f82`/5F7B |
| 6046 | `nemici_tipo07b.c` r. 332 | in `routine_tab.c`, 13 catture, 2 non forzate |
| 6FC3 | `nemici_tipi_08_14.c` r. 423 | `static`, raggiunta da 6F91 (forzate `stanza08_forza6fc3`) |
| 6FDF | `nemici_tipi_08_14.c` r. 443 | `static`, idem (`stanza08_forza6fdf`) |

Per la prova: «mai osservate» dalle sessioni di PC (`copertura_call.py`) non
vuol dire «non tradotte»: queste cinque hanno cattura forzata o tradotta
dal dasm. Le lasci fuori dal conteggio dei 59 ma le elenchi in una tabella
`TRADOTTA` (5 righe).

### Le 59 rimaste (misurato: `grep -v '^#' decomp/out/call_mai_osservate.txt | cut -f1 | grep -v -E '^(4FC5|5F82|6046|6FC3|6FDF)$' | wc -l` → `59`)

Sotto 8000 (22): `0071 029D 09A2 1020 1604 17A4 2114 2533 2FC3 3254 3504
4364 4A05 540A 5564 5604 560A 570A 6379 68B0 7E03 7E14`.
A 8000 o sopra (37): `8010 801E 8514 856C 8719 9111 9557 982C A001 A804 A904
AB04 AB14 AEE2 AEF6 AF00 AF0A AF14 AF1E AF28 AF32 AF3C AF46 AF50 AF5A AF64
AF6E AFA0 AFAA AFB4 AFBE AFC8 AFD2 AFDC AFE6 AFF0 BE2B`.

Siti di chiamata (colonna 2 di `call_mai_osservate.txt`), dato che orienta
l'analisi: 44 delle 64 hanno il sito a 8000 o sopra (cioè dentro la zona
80A5-BFFF probabilmente dati); i siti sotto 8000 delle 59 rimaste sono:
2114←1330, 2533←6879, 4364←64A8, 5564←647C, 6379←6310, 7E03←7DA0, 7E14←7DC6,
8514←1D25, 856C←1EA1, 8719←1EDF, 9111←1C0B, 9557←57DC, 982C←3B1F, AB14←4FAD,
BE2B←2D83. Sono i candidati a «codice vero che chiama in zona dati» o a
falso codice dentro tabelle: da guardare per primi.

Indizi già raccolti (da verificare, non da copiare): `2114` compare in
`rst20_tables.h` r. 131-132 come parola di una tabella (quindi il valore
2114 è anche dato); `1020` in `attivita.c` r. 43 è `ld bc,1020` (immediato,
non una destinazione); `560A` in `nemici_tipi_10_13.c` r. 78 è un commento
d'indirizzo dentro un'altra routine; `4364` in `z80_4xxx.c` r. 504 è un
intervallo di commento `4361-4364`; `0071` in `audio_dati_spartiti_f.c` e
`029D` in `audio_interprete.c` sono dati dell'audio (altro Z80, altra ROM,
non conta). Ognuno di questi è una pista, non un verdetto: confermare dal
dasm e dalla RAM/ROM.

## Metodo (in quest'ordine, costo crescente)

1. **Dato o codice**, senza MAME. Per ogni destinazione estrarre dal
   `main64.dasm` le righe intorno al sito di chiamata e alla destinazione
   (`grep -n ';SITO' decomp/out/main64.dasm`, poi poche righe: mai file
   interi). Indizi di DATO: il sito è in una zona che non è mai eseguita
   né raggiunta da nessuna `call`/`jp` eseguita; le istruzioni intorno non
   formano codice sensato (`rst` consecutivi, `ld b,h`, `and a` a
   catena, byte che sono coordinate o puntatori); la destinazione cade
   dentro una tabella indicizzata (puntatori a parole, passo regolare);
   per ≥8000 i byte sono riferiti da una tabella di livelli o di testi
   (vedi `DATI.md`, `estrai_dati.py`). Mostrare il perché con 1-3 righe
   di dasm o di byte esadecimali nel documento.
2. **Codice morto**. Se il sito e la destinazione sembrano codice vero ma
   non c'è alcuna via d'ingresso al sito: cercare chi chiama o salta
   al blocco che contiene il sito (`grep` del suo indirizzo nel dasm,
   anche come parola in tabelle); se nessuno, MORTO con la catena
   «sito ← blocco ← nessun riferimento». Nel dasm statico un ingresso da
   `jp (hl)` o da tabella di parole conta come riferimento: cercare la
   parola little-endian `XX YY` nella ROM (Python con `out/main64.bin`).
3. **Raggiungibile** se il blocco del sito ha un chiamante eseguito e un
   ramo condizionale non preso (come 4FC5/fata): scrivere la condizione
   (flag, byte di RAM, evento di gioco) e provare la forzatura:
   - copia di lavoro: `mkdir -p /tmp/z40/out`, collegare `roms`, copiare
     o collegare gli script (non modificarli), `SNAP_DIR=/tmp/z40/out`;
   - cattura con `sh cattura.sh 0x<DEST> <stato> 1 z40` (stato `gioco`,
     `tardo`, `stanzaNN`, `attNN`...), eventualmente con `SNAP_POKE`,
     `SNAP_POSTA` o `SNAP_FORZA` per accendere la condizione; una
     cattura riuscita stampa `CATTURA <ENTRY> <tag> frame_da=... OK`
     (da `decomp/` copiato; una cattura dura 40-100 s, fino a 6 in
     parallelo con `xargs -P6`);
   - tetto di costo: al massimo 3 tentativi per destinazione e circa 40
     minuti di MAME in totale; oltre, scrivere «non svegliata» con i
     tentativi fatti e passare oltre. Non tradurre nulla, non committare
     catture.
   Forzare il PC direttamente a un indirizzo non vale come prova di
   raggiungibilità (dimostra al più che i byte sono eseguibili): se lo si
   usa, dirlo nella riga come «PC forzato, non raggiungibile per via
   naturale» e il verdetto resta MORTO o DATO.
4. Il censimento di cui si fa solo scrittura è `CENSIMENTO-12.md`
   (sezione indicata sopra): la colonna `sito_eseguito` resta, si
   aggiungono una colonna `verdetto` e una `note` (o un rimando a
   `ZONE-MAI-ESEGUITE.md`), e le 5 tradotte si segnano «tradotta»
   (titolo della sezione: «Destinazioni ancora scoperte (59 + 5
   tradotte)» o simile; il numero 64 non deve restare come conteggio
   delle scoperte).

## Struttura del documento `decomp/ZONE-MAI-ESEGUITE.md`

Italiano. Sezioni, nell'ordine:

1. `# Destinazioni mai eseguite del Z80 principale`: premessa di 8-10
   righe (da dove viene l'elenco: `copertura_call.py --classifica`,
   299 destinazioni, 64 mai osservate, 5 tradotte, 59 verdetti;
   significato dei tre verdetti; cosa vuol dire «sito mai eseguito»).
2. `## Verdetti`: **una sola tabella** con una riga per destinazione,
   in ordine di indirizzo, colonne esattamente:
   `| destinazione | verdetto | sito | motivo | ricetta |`.
   Righe nel formato `| 0071 | DATO | 85EA | <motivo breve> | - |`:
   - destinazione e sito: 4 cifre esadecimali maiuscole;
   - verdetto: esattamente una di `DATO`, `MORTO`, `RAGGIUNGIBILE`;
   - motivo: una frase con i riferimenti (indirizzi, tabella, byte);
   - ricetta: `-` salvo RAGGIUNGIBILE; per le provate la riga di comando
     completa e il frame/esito, per le non provate «non svegliata:
     <tentativi>» e la condizione teorica.
   Nessun'altra riga di tabella in questa sezione che cominci con
   `| XXXX |` (le verifiche contano le righe).
3. `## Già tradotte`: le 5 righe `| 4FC5 | TRADOTTA | ... |` con file e
   rimando (copiare la tabella sopra, controllando che sia ancora vero).
4. `## Note`: raggruppamenti (per esempio la famiglia AEE2-AFF0: passo
   regolare di 10 byte e siti A88E-AA80 ⇒ tabella di record), cosa resta
   davvero da svegliare, ricette provate, ipotesi non verificate marcate
   come tali. Se una zona ha un'unica spiegazione (per esempio 37
   destinazioni ≥8000 sono tutte dentro dati di livello), scriverla una
   volta e rimandare dalle righe.
5. `## Come si riproduce`: i comandi esatti (copertura_call, grep del
   dasm) per rigenerare l'elenco, con la cartella di uscita scratch.

Un verdetto per destinazione, non per gruppo: se un gruppo condivide il
motivo, la riga ripete il motivo in breve e le Note spiegano il gruppo.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master`. Prima di toccare qualsiasi cosa:
`md5sum decomp/out/call_mai_osservate.txt decomp/out/eseguiti_unione.txt decomp/out/main48.dasm decomp/out/main64.dasm > /tmp/z40_md5_prima.txt`.

1. Un verdetto per destinazione: `grep -c -E '^\| [0-9A-F]{4} \| (DATO|MORTO|RAGGIUNGIBILE) \|' decomp/ZONE-MAI-ESEGUITE.md`
   → `59`.
2. Insieme esatto: `diff <(grep -o -E '^\| [0-9A-F]{4} \| (DATO|MORTO|RAGGIUNGIBILE) \|' decomp/ZONE-MAI-ESEGUITE.md | awk '{print $2}' | sort) <(grep -v '^#' decomp/out/call_mai_osservate.txt | cut -f1 | grep -v -E '^(4FC5|5F82|6046|6FC3|6FDF)$' | sort) && echo identico`
   → `identico`.
3. Nessun verdetto doppio: `grep -E '^\| [0-9A-F]{4} \| (DATO|MORTO|RAGGIUNGIBILE) \|' decomp/ZONE-MAI-ESEGUITE.md | awk '{print $2}' | sort | uniq -d | wc -l`
   → `0`. Tradotte: `grep -c -E '^\| [0-9A-F]{4} \| TRADOTTA \|' decomp/ZONE-MAI-ESEGUITE.md` → `5`.
4. Nessuna cella vuota o segnaposto: `grep -E '^\| [0-9A-F]{4} \| (DATO|MORTO|RAGGIUNGIBILE|TRADOTTA) \|' decomp/ZONE-MAI-ESEGUITE.md | grep -c -i -E 'da verificare|TODO|\|\s*\|'`
   → `0`.
5. Censimento aggiornato: `grep -c 'Destinazioni ancora scoperte (64)' decomp/CENSIMENTO-12.md`
   → `0` (oggi `1`); `grep -c 'ZONE-MAI-ESEGUITE' decomp/CENSIMENTO-12.md` → almeno `1`.
6. Dati di ingresso intatti: `md5sum -c /tmp/z40_md5_prima.txt` → quattro `OK`.
7. Nessun file di codice, banco o script toccato:
   `git status --short` → solo righe `?? decomp/ZONE-MAI-ESEGUITE.md`,
   ` M decomp/CENSIMENTO-12.md` e, se usato, ` M decomp/ROUTINE.md`
   (e `?? compiti/40-` se il brief non è ancora committato);
   `git status --short | grep -v -E 'ZONE-MAI-ESEGUITE|CENSIMENTO-12|ROUTINE\.md|compiti/40-'` → vuoto;
   `git status --short | grep -c -E '\.(gcda|gcno|c|h|lua|py|sh)$'` → `0`.
8. Banco invariato (misurato il 2026-11-03): `cd decomp/harness && ./harness --tutte > /tmp/h40_dopo.txt; echo "exit $?"; grep -c FALLITO /tmp/h40_dopo.txt; grep -c '^RIEPILOGO' /tmp/h40_dopo.txt; grep -c '^cattura' /tmp/h40_dopo.txt`
   → `exit 0`, `0`, `304`, `1270`.
9. `sched_proto` (da `decomp/`): `harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
10. Replay (da `decomp/`): `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r40.txt 2>&1; echo "exit $?"; tail -4 /tmp/r40.txt` →
    `exit 0` e (misurato):
    ```
    spedizioni eseguite: 64686, fino al frame 5499
    NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
    prima divergenza a metà attività (attesa): frame 2415
    nessuna divergenza nello spedidore
    ```
11. Nessun MAME residuo né con finestra: `pgrep -c mame` → `0`
    (la copia di lavoro in `/tmp/z40` può restare, non fa parte della
    consegna).
12. Dimensioni: `wc -l decomp/ZONE-MAI-ESEGUITE.md | awk '$1>800'` → vuoto.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto 40: le 59 destinazioni mai eseguite documentate (dato, morto, raggiungibile)`
  e riga finale `Co-Authored-By: <modello> <noreply@anthropic.com>`.
  Comprende `decomp/ZONE-MAI-ESEGUITE.md`, `decomp/CENSIMENTO-12.md`
  (ed eventualmente `decomp/ROUTINE.md`); nient'altro.
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-12;
  conteggio dei verdetti per tipo (`DATO`/`MORTO`/`RAGGIUNGIBILE`); elenco
  delle ricette provate con esito; destinazioni RAGGIUNGIBILI non
  svegliate, con il motivo; minuti di MAME spesi; deviazioni dal brief.

## Domande aperte

1. **Le 5 tradotte fuori dal conteggio.** Il perimetro dei verdetti è 59
   e le 5 (4FC5, 5F82, 6046, 6FC3, 6FDF) stanno in una tabella a parte.
   Proposta: come scritto. — «preferenza»
2. **Nome e posto del documento**: `decomp/ZONE-MAI-ESEGUITE.md`, come
   chiesto; tabella unica e verdetti in maiuscolo. Proposta: come scritto.
   — «preferenza»
3. **Tetto di MAME**: 3 tentativi per destinazione e ~40 minuti totali
   per le ricette. Se le RAGGIUNGIBILI fossero molte (oltre una decina) il
   tetto non basterebbe a svegliarle tutte. Proposta: tetto come scritto;
   le non svegliate restano RAGGIUNGIBILE con «non svegliata». —
   «preferenza»
4. **Censimento 12 modificato nella sola sezione delle scoperte.**
   `CENSIMENTO-NEMICI.md` e `STATO.md` restano fuori (STATO lo aggiorna la
   chat principale a lotto chiuso, con il risultato). Proposta: come
   scritto. — «preferenza»
5. **Cosa conta come RAGGIUNGIBILE per i falsi codici in zona dati.** Un
   blocco che le sessioni non eseguono ma che potrebbe essere eseguito
   solo se un puntatore di livello/tabella lo selezionasse (per esempio
   famiglia AEE2-AFF0 se fosse una tabella di funzioni) può cadere fra
   DATO e RAGGIUNGIBILE. Proposta: DATO se l'unico riferimento è una
   tabella di dati letta da codice che non la salta mai; RAGGIUNGIBILE solo
   se esiste un `jp (hl)`/`call (hl)`/`rst` eseguibile che usa la tabella
   come indirizzi di codice. — «preferenza»
6. **Uso di una partita dal vivo.** Un'eventuale ricetta che richieda un
   evento di gioco non riproducibile con RAM e poste (per esempio una
   sequenza di input lunga) non si prova a mano e `copertura_viva.lua`
   non si rilancia con finestra. Se per qualche destinazione quel solo
   modo la sveglierebbe, andrebbe deciso se chiedere una partita
   all'utente. Proposta: segnarla RAGGIUNGIBILE con «richiede partita dal
   vivo» e rinviare. — «scelta di progetto»

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domande 1-5 sono preferenze, accolte con la proposta dell'analista:
1. Sì: le 5 già tradotte (4FC5, 5F82, 6046, 6FC3, 6FDF) in tabella a parte, fuori dal conteggio dei verdetti.
2. Sì: documento `decomp/ZONE-MAI-ESEGUITE.md`, tabella unica con verdetti `DATO`, `MORTO`, `RAGGIUNGIBILE`.
3. Sì: tetto MAME di 3 tentativi per destinazione e ~40 minuti in tutto; le non svegliate restano `RAGGIUNGIBILE` con nota «non svegliata».
4. Sì: si modifica solo `CENSIMENTO-12.md`; `STATO.md` e `CENSIMENTO-NEMICI.md` restano fuori (STATO lo aggiorna la chat principale).
5. Sì: falso codice in zona dati = `DATO` se l'unico riferimento è una tabella letta come dato; `RAGGIUNGIBILE` solo se un salto indiretto eseguibile usa la tabella come indirizzi di codice.

Domanda 6 (scelta di progetto, risolta dalla chat principale sotto il mandato
dell'utente e coerente con la sua preferenza nota «niente finestre MAME a
ripetizione»): sì, le destinazioni che si svegliano solo con una partita dal
vivo si segnano `RAGGIUNGIBILE` con nota «richiede partita dal vivo» e si
rinviano, senza rilanciare `copertura_viva.lua`. Da segnalare all'utente nel
rapporto di fine ciclo.

## Esito (chat principale, 2026-11-03)

Passato al primo colpo, dodicesimo ciclo con lo schema economico
(analista Sonnet ~74k token, sviluppatore Sonnet ~154k, verifica a
script senza modello). Le 59 destinazioni hanno verdetto in
`decomp/ZONE-MAI-ESEGUITE.md`: 53 `DATO`, 0 `MORTO`, 6 `RAGGIUNGIBILE`
di cui 5 esercitate con ricetta provata (2533, 6379, 4364 via forzata
esistente, 7E03/7E14 coi dip switch Bonus Life) e 560A non svegliata
(serve un nemico di tipo 12 con via libera; 0 tentativi, motivato).
Banco, replay e `sched_proto` INTATTI (304 voci, 1270 catture, 0
fallite); nessun file di codice toccato. MAME ~1 minuto in tutto,
sempre senza finestra; la clausola «partita dal vivo» (scelta di
progetto risolta dalla chat) non è mai scattata. Scoperta collaterale
da annotare: la tabella `rst 20h` a 646B ha 10 parole vere contro le
3 di `rst20_tables.h` (file non corretto per vincolo del brief, tutte
le 10 già eseguite).
