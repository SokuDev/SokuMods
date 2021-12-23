# SokuStreaming
Streaming tool for Touhou 12.3: Hisoutensoku. SokuStreaming provides an easy to use API to
fetch information from the game and connect to games.

# Documentation
## Routes
### /
Accepted methods: GET

Note: Always redirects to the default overlay at /static/html/overlay.html
#### Response Code
- 405 Method Not Allowed
- 301 Moved Permanently

### /state
Accepted methods: GET, POST

GET -> Returns a **State** object describing the current state of the game.

POST -> Takes a **PartialState** object and updates the state accordingly.
Nothing is returned.

Note that any POST to this route that doesn't come from 127.0.0.1 will result in a 403.

#### Response Code
- 400 Bad Request: The given JSON object was invalid. (POST only)
- 403 Forbidden (POST only)
- 405 Method Not Allowed
- 200 OK

### /connect
Accepted methods: POST

POST -> Takes a **ConnectionRequest** object and tries to connect to the specified game.
Nothing is returned.

Note that any call to this route that doesn't come from 127.0.0.1 will result in a 403.

#### Response Code
- 400 Bad Request: The given JSON object was invalid.
- 403 Forbidden
- 405 Method Not Allowed
- 503 Service Unavailable: The game is not ready to connect (either hosting or already connecting)
- 202 Accepted

### /static/*
Accepted methods: GET

These routes map directly to the static folder in the mod folder. If you want to access
the file at `modules/SokuStreaming/static/image.png`, you can get it from the route
`/static/image.png`

#### Response Code
- 404 Not Found
- 405 Method Not Allowed
- 200 OK

### /chat
Starts a websocket connection to the game. See the Websocket section for more details.

#### Response Code
- 400 Bad Request
- 101 Switching Protocols

## Websocket
The websocket is used to communicate events to the connected client about the game state.
All client messages are ignored.

A websocket event looks like this.
```JSON
{
    "o": 2,
    "d": 1
}
```
o: Integer -> Opcode

d: &lt;Anything&gt; -> The data associated with the opcode.

### Opcodes
#### STATE_UPDATE (0)
Data type -> State object

Indicate a big change in the state. The data is the newly updated state.
#### CARDS_UPDATE (1)
Data type -> CardUpdate object

Indicate a big change in the state. The data is the newly updated state.
#### L_SCORE_UPDATE (2)
Data type: Integer

Indicate a change of score for the left player. The data is the newly updated score.
#### R_SCORE_UPDATE (3)
Data type: Integer

Indicate a change of score for the right player. The data is the newly updated score.
#### L_CARDS_UPDATE (4)
Data type: List[Integer]

Indicate a change of deck state for the left player. The data is the newly updated deck.
#### R_CARDS_UPDATE (5)
Data type: List[Integer]

Indicate a change of deck state for the right player. The data is the newly updated deck.
#### L_NAME_UPDATE (6)
Data type: String

Indicate a change of name for the left player. The data is the newly updated name.
#### R_NAME_UPDATE (7)
Data type: String

Indicate a change of name for the right player. The data is the newly updated name.
#### L_STATS_UPDATE (8)
Data type: Stats object

Indicate a change of statistics for the left player. The data is the newly updated stats.
#### R_STATS_UPDATE (9)
Data type: Stats object

Indicate a change of statistics for the right player. The data is the newly updated stats.
#### GAME_ENDED (10)
Data type: None

The current game just ended.
#### GAME_STARTED (11)
Data type: None

The current game just started.
#### SESSION_ENDED (12)
Data type: None

A new game session just started.
#### SESSION_STARTED (13)
Data type: None

A new game session just ended.
## Data
### <u>PartialState</u> object
```JSON
{
    "left": {
        "name": "PinkySmile",
        "score": 0
    },
    "right": {
        "name": "SokuAI",
        "score": 9
    },
    "round": "Grand Final"
}
```
left.name: String -> New name of the left player (P1)

left.score: String -> New score of the left player (P1)

right.name: String -> New name of the right player (P2)

right.score: String -> New score of the right player (P2)

round: String -> New round string

### <u>State</u> object
```JSON
{
    "isPlaying": true,
    "left": {
        "character": 11,
        "deck": [
            0,
            12,
            13,
            100,
            100
        ],
        "hand": [
            110,
            200,
            200,
            211,
            211
        ],
        "name": "Langschlafer",
        "palette": 7,
        "score": 1,
        "stats": {
            "doll": 0,
            "drops": 0,
            "fan": 0,
            "grimoire": 0,
            "rod": 0,
            "skills": {
                "0": 2,
                "1": 0,
                "2": 0,
                "3": 0
            },
            "special": 0
        },
        "used": [
            100,
            100,
            200,
            202,
            202,
            202,
            205,
            205,
            205,
            211
        ]
    },
    "right": {
        "character": 18,
        "deck": [
            4,
            105,
            105,
            108,
            108,
            108,
            213
        ],
        "hand": [
            4,
            15,
            105,
            108,
            211
        ],
        "name": "ViLi",
        "palette": 3,
        "score": 0,
        "stats": {
            "doll": 0,
            "drops": 0,
            "fan": 0,
            "grimoire": 0,
            "rod": 0,
            "skills": {
                "0": 0,
                "2": 0,
                "3": 0,
                "5": 1
            },
            "special": 0
        },
        "used": [
            4,
            4,
            105,
            201,
            203,
            203,
            212,
            213
        ]
    },
    "round": ""
}
```
isPlaying: Boolean -> Whether the game is in progress or not.

left: CharacterState -> Left player (P1) state

right: CharacterState -> Right player (P2) state

round: String -> The round string set either in game or doing a POST at /state

If "isPlaying" is false, the left and right character states are **not** guaranteed to be valid.

### <u>CharacterState</u> object
```JSON
{
    "character": 18,
    "deck": [
        4,
        105,
        105,
        108,
        108,
        108,
        213
    ],
    "hand": [
        4,
        15,
        105,
        108,
        211
    ],
    "name": "Seiki",
    "palette": 3,
    "score": 0,
    "stats": {
        "doll": 0,
        "drops": 0,
        "fan": 0,
        "grimoire": 0,
        "rod": 0,
        "skills": {
            "0": 0,
            "2": 0,
            "3": 0,
            "5": 1
        },
        "special": 0
    },
    "used": [
        4,
        4,
        105,
        201,
        203,
        203,
        212,
        213
    ]
}
```
character: Integer -> Character ID. A list can be found [here](https://github.com/SokuDev/SokuLib/blob/29ef922a58902754f9d4a45d5e57e35cc35f1510/src/Character.hpp#L19).

deck: List[Integer] -> List of card ID that are left in the deck.

hand: List[Integer] -> List of card ID that are in the player's hand.

name: String -> Name of the profile. If it didn't get changed using /state or
the in game keys, this is the profile name.

palette: Integer -> The palette the player chose from 0 to 7 (0 is the default).

score: Integer -> The current score of the player in the set. Increased when a game is won.

stats: Stats -> Statistics modifiers and skills.

used: List[Integer] -> List of card ID that were used by the player.

Note that when Mountain Vapor activates, the hand will always be empty and all the deck
will be replaced with the "21" card ID indicating they are hidden (21 is not a valid card
id). When the weather returns to normal, all cards can be seen again.

### <u>Stats</u> object
```JSON
{
    "doll": 0,
    "drops": 0,
    "fan": 0,
    "grimoire": 0,
    "rod": 0,
    "skills": {
        "0": 0,
        "2": 0,
        "3": 0,
        "5": 1
    },
    "special": 0
}
```
stats.doll: Integer -> The number of Sacrificial Doll system card used.

stats.drops: Integer -> The number of Heavenly Drop system card used.

stats.fan: Integer -> The number of Tengu Fan system card used.

stats.grimoire: Integer -> The number of Grimoire system card used.

stats.rod: Integer -> The number of Control Rod system card used.

stats.skills: Skills object -> The current skills the player have and their level.

stats.special: Integer -> Indicate the number of other stackablez character specific card.
The supported card are Reisen's Elixir and Yuyuko's Resurrection Butterflies.

### <u>Skills</u> object
```JSON
{
    "0": 0,
    "2": 0,
    "3": 0,
    "5": 1
}
```
A pair of key/value indicating which skill the player has and at which level it is.
There must always be 4 pairs except for Patchouli (and Kaguya when using Soku2) which have 5.
The key value is the skill ID and the value is its level. Value from 0-3 (0-4 for Patchy)
are default skills, 4-7 (5-9 for Patchy) are first set of alt skills and 8-11 (10-14 for Patchy)
are the last set of alt skills. The skill input is always in the same order in between
alts and default but changes depending on the character. You can find the skill order
by looking at the skill card IDs: subtract 100 from the card id and you will get the
corresponding skill ID.