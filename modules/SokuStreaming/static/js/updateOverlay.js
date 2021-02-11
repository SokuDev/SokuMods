let sokuCharacters = [
    {"name": 'reimu',     "skillPrefix": "Ha", "skills": [236, 214, 421, 623]},
    {"name": 'marisa',    "skillPrefix": "Ma", "skills": [214, 623, 22,  236]},
    {"name": 'sakuya',    "skillPrefix": "Iz", "skills": [623, 214, 236, 22 ]},
    {"name": 'alice',     "skillPrefix": "Al", "skills": [236, 623, 214, 22 ]},
    {"name": 'patchouli', "skillPrefix": "Pa", "skills": [236, 22,  623, 214, 421]},
    {"name": 'youmu',     "skillPrefix": "Yo", "skills": [236, 623, 214, 22 ]},
    {"name": 'remilia',   "skillPrefix": "Re", "skills": [236, 214, 623, 22 ]},
    {"name": 'yuyuko',    "skillPrefix": "Ji", "skills": [214, 236, 421, 623]},
    {"name": 'yukari',    "skillPrefix": "Yu", "skills": [236, 623, 214, 421]},
    {"name": 'suika',     "skillPrefix": "Ib", "skills": [236, 623, 214, 22 ]},
    {"name": 'reisen',    "skillPrefix": "Ud", "skills": [236, 214, 623, 22 ]},
    {"name": 'aya',       "skillPrefix": "Ay", "skills": [236, 214, 22,  421]},
    {"name": 'komachi',   "skillPrefix": "Ko", "skills": [236, 623, 22,  214]},
    {"name": 'iku',       "skillPrefix": "Ik", "skills": [236, 623, 22,  214]},
    {"name": 'tenshi',    "skillPrefix": "Te", "skills": [214, 22,  236, 623]},
    {"name": 'sanae',     "skillPrefix": "Sa", "skills": [236, 22,  623, 214]},
    {"name": 'cirno',     "skillPrefix": "Ci", "skills": [236, 214, 22,  623]},
    {"name": 'meiling',   "skillPrefix": "Me", "skills": [214, 623, 22,  236]},
    {"name": 'utsuho',    "skillPrefix": "Ut", "skills": [623, 236, 22,  214]},
    {"name": 'suwako',    "skillPrefix": "Sw", "skills": [214, 623, 236, 22 ]},
];
let global_state = null;
let json = {};
let Opcodes = {
    "STATE_UPDATE":   0,
    "CARDS_UPDATE":   1,
    "L_SCORE_UPDATE": 2,
    "R_SCORE_UPDATE": 3,
    "L_CARDS_UPDATE": 4,
    "R_CARDS_UPDATE": 5,
    "L_NAME_UPDATE":  6,
    "R_NAME_UPDATE":  7,
    "L_STATS_UPDATE": 8,
    "R_STATS_UPDATE": 9,
}

function pad(n, width, z) {
    z = z || '0';
    n = n + '';
    return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
}

function getCharacterImage(id)
{
    return "/static/img/characters/" + sokuCharacters[id].name + ".png"
}

function getCardImage(charId, cardId)
{
    if (cardId < 100)
        return "/static/img/cards/system/Soku_common_card" + pad(cardId, 3) + ".png";

    let cardType;
    let chr = sokuCharacters[charId];

    if (cardId < 200) {
        let id = cardId - 100;

        cardType = chr.skills[id % chr.skills.length] + "bc" + Math.floor(id / chr.skills.length + 1);
    } else
        cardType = "sc" + pad(cardId - 200, 2);
    return "/static/img/cards/" + chr.name + "/" + chr.skillPrefix + cardType + ".png"
}

function getStatsImage(charId, statId)
{
    let chr = sokuCharacters[charId];
    let cardType = chr.skills[statId % chr.skills.length] + "bc" + Math.floor(statId / chr.skills.length + 1);

    return "/static/img/stats/" + chr.name + "/" + chr.skillPrefix + cardType + ".png"
}

function updateStat(charId, id, stats, stat)
{
    let nb = stats[stat];
    let div = document.getElementById(id + stat);

    if (nb === 0) {
        div.style.height = "0";
        return;
    }
    div.style.height = "";
    document.getElementById(id + stat + "lvl").setAttribute("src", "/static/img/stats/LEVEL00" + nb + ".png");
}

function displayStats(charId, id, stats)
{
    let chr = sokuCharacters[charId];

    updateStat(charId, id, stats, "rod");
    updateStat(charId, id, stats, "doll");
    updateStat(charId, id, stats, "fan");
    updateStat(charId, id, stats, "grimoire");
    updateStat(charId, id, stats, "drops");
    for (let i = chr.skills.length; i < 5; i++) {
        document.getElementById(id + "skill" + i).style.height = "0";
    }
    for (const [key, value] of Object.entries(stats["skills"])) {
        let div = document.getElementById(id + "skill" + (key % chr.skills.length));
        let icon = document.getElementById(id + "skill" + (key % chr.skills.length) + "icon");
        let level = document.getElementById(id + "skill" + (key % chr.skills.length) + "lvl");

        if (value) {
            div.style.height = "";
            icon.setAttribute("src", getStatsImage(charId, key));
            level.setAttribute("src", "/static/img/stats/LEVEL00" + value + ".png");
        } else {
            div.style.height = "0";
        }
    }
}

function displayDeck(id, used, hand, deck, chr)
{
    let i = 0;

    for (let g = 0; i < 20 && g < hand.length; i++) {
        let img = document.getElementById(id + i);
        let src = getCardImage(chr, hand[g]);

        img.setAttribute("src", src);
        img.className = "hand_card";
        g++;
    }
    for (let g = 0; i < 20 && g < deck.length; i++) {
        let img = document.getElementById(id + i);
        let src = getCardImage(chr, deck[g]);

        img.setAttribute("src", src);
        img.className = "card";
        g++;
    }
    for (let g = 0; i < 20 && g < used.length; i++) {
        let img = document.getElementById(id + i);
        let src = getCardImage(chr, used[g]);

        img.setAttribute("src", src);
        img.className = "used_card";
        g++;
    }
    for (; i < 20; i++) {
        let img = document.getElementById(id + i);

        img.className = "unused_card";
    }
}

function checkState()
{
    if (global_state)
        return true;

    const Http = new XMLHttpRequest();
    const url = '/state';

    console.warn("State is not initialized...");
    Http.open("GET", url);
    Http.send();
    Http.onload = update
    return false;
}

function update(state)
{
    global_state = state;

    let lchr = state.left.character;
    let rchr = state.right.character;
    let round = document.getElementById("roundMarker");

    document.getElementById("lChr").setAttribute("src", getCharacterImage(lchr));
    document.getElementById("rChr").setAttribute("src", getCharacterImage(rchr));
    document.getElementById("leftName").textContent = state.left.name;
    document.getElementById("rightName").textContent = state.right.name;
    document.getElementById("leftScore").textContent = state.left.score + "";
    document.getElementById("rightScore").textContent = state.right.score + "";

    if (round)
        round.textContent = state.round;

    displayDeck("lCard", state.left.used,  state.left.hand,  state.left.deck,  lchr);
    displayDeck("rCard", state.right.used, state.right.hand, state.right.deck, rchr);
    displayStats(lchr, "l", state.left.stats);
    displayStats(rchr, "r", state.right.stats);
}

function updateDecks(decks)
{
    if (!checkState())
        return;

    displayDeck("lCard", decks.left.used,  decks.left.hand,  decks.left.deck,  global_state.left.character);
    displayDeck("rCard", decks.right.used, decks.right.hand, decks.right.deck, global_state.right.character);

    global_state.left.deck  = decks.left.deck;
    global_state.left.used  = decks.left.used;
    global_state.left.hand  = decks.left.hand;
    global_state.right.deck = decks.right.deck;
    global_state.right.used = decks.right.used;
    global_state.right.hand = decks.right.hand;
}

function updateLeftScore(newScore) {
    if (!checkState())
        return;
    document.getElementById("leftScore").textContent = newScore + "";
    global_state.left.score = newScore;
}

function updateRightScore(newScore)
{
    if (!checkState())
        return;
    document.getElementById("rightScore").textContent = newScore + "";
    global_state.right.score = newScore;
}

function updateLeftDeck(deck)
{
    if (!checkState())
        return;
    displayDeck("lCard", deck.used,  deck.hand,  deck.deck,  global_state.left.character);
    global_state.left.deck  = deck.deck;
    global_state.left.used  = deck.used;
    global_state.left.hand  = deck.hand;
}

function updateRightDeck(deck)
{
    if (!checkState())
        return;
    displayDeck("rCard", deck.used, deck.hand, deck.deck, global_state.right.character);
    global_state.right.deck = deck.deck;
    global_state.right.used = deck.used;
    global_state.right.hand = deck.hand;
}

function updateLeftStats(stats)
{
    if (!checkState())
        return;
    displayStats(global_state.left.character, "l", stats);
    global_state.left.stats = stats;
}

function updateRightStats(stats)
{
    if (!checkState())
        return;
    displayStats(global_state.right.character, "r", stats);
    global_state.right.stats = stats;
}

function updateLeftName(newName)
{
    if (!checkState())
        return;
    document.getElementById("leftName").textContent = newName;
    global_state.left.name = newName;
}

function updateRightName(newName)
{
    if (!checkState())
        return;
    document.getElementById("rightName").textContent = newName;
    global_state.right.name = newName;
}

function handleWebSocketMsg(event)
{
    let json = JSON.parse(event.data);
    let data = json.d;

    console.log(json);
    switch (json.o) {
    case Opcodes.STATE_UPDATE:
        return update(data);
    case Opcodes.CARDS_UPDATE:
        return updateDecks(data);
    case Opcodes.L_SCORE_UPDATE:
        return updateLeftScore(data);
    case Opcodes.R_SCORE_UPDATE:
        return updateRightScore(data);
    case Opcodes.L_CARDS_UPDATE:
        return updateLeftDeck(data);
    case Opcodes.R_CARDS_UPDATE:
        return updateRightDeck(data);
    case Opcodes.L_NAME_UPDATE:
        return updateLeftName(data);
    case Opcodes.R_NAME_UPDATE:
        return updateRightName(data);
    case Opcodes.L_STATS_UPDATE:
        return updateLeftStats(data);
    case Opcodes.R_STATS_UPDATE:
        return updateRightStats(data);
    }
}

function initWebSocket() {
    let url = "ws://" + window.location.href.split('/')[2] + "/chat";

    console.log("Connecting to " + url);

    let sock = new WebSocket(url);

    sock.onmessage = handleWebSocketMsg;
    sock.onclose = (e) => {
        console.warn(e);
        global_state = null;
        setTimeout(initWebSocket, 10000);
    };
    sock.onerror = console.error;
}
initWebSocket();