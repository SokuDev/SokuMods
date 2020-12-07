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
let json = {};

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

function realUpdate(e)
{
    let j = JSON.parse(e.target.response);
    let lchr = json.left.character;
    let rchr = json.right.character;

    console.log(json);
    document.getElementById("lChr").setAttribute("src", getCharacterImage(lchr));
    document.getElementById("rChr").setAttribute("src", getCharacterImage(rchr));
    document.getElementById("leftName").textContent = j.ln || json.left.name;
    document.getElementById("rightName").textContent = j.rn || json.right.name;
    document.getElementById("leftScore").textContent = j.ls + json.left.score + "";
    document.getElementById("rightScore").textContent = j.rs +json.right.score + "";

    displayDeck("lCard", json.left.used, json.left.hand, json.left.deck, lchr);
    displayDeck("rCard", json.right.used, json.right.hand, json.right.deck, rchr);
}

function update(e)
{
    json = JSON.parse(e.target.response);

    const Http = new XMLHttpRequest();
    const url = '/static/override.json';

    Http.open("GET", url);
    Http.send();
    Http.onload = realUpdate
}

function initiateUpdate() {
    const Http = new XMLHttpRequest();
    const url = '/state';

    Http.open("GET", url);
    Http.send();
    Http.onload = update
}

initiateUpdate()
//setInterval(initiateUpdate, 500);

let sock = new WebSocket("ws://" + window.location.href.split('/')[2] + "/chat");

sock.onmessage = console.log;
sock.onclose = console.warn;
sock.onerror = console.error;