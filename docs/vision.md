# Vision (working title: "Codex", codename only)

> "Codex" collides with an existing product name. Pick a real title and run a trademark search before any public release.

## One line
A grounded, bittersweet driving journey across a very large real-world map in real time: restore your grandfather's car, build a trucking business, and finish his unfinished trip, with a camera in your hand.

## North star
The most believable long drive ever made. You trust the car, the road feels real, and the journey stays with you. Every feature must make the drive more believable, more present, or more memorable; otherwise it goes to the parking lot (`docs/ideas.md`).

## Setting and tone
Present day (2026-2027). Realistic, grounded, **bittersweet and warm**. Understated writing, unglamorous beauty (golden hour on a dusty road, a thermos on the passenger seat), small stakes treated honestly. No stunt or arcade tone.

## Two modes (chosen from the main menu)
- **Free Mode:** the open world, everything unlocked. Pick any vehicle, region, time and weather. Drive, photograph, explore.
- **Career Mode:** begins with the journey. A grandson (a photographer) inherits his grandfather's rusty car and a tin of maps and photos. Broke, he takes cargo jobs, repairs the car piece by piece, builds a trucking business, finds barn-find projects, and follows his grandfather's route across the world.
Both modes use the same simulation and world. Both support multiplayer (soft multiplayer first).

## Pillars
1. **Presence:** cockpit, inertia, audio, weather on glass, light.
2. **Journey:** real-time distance, navigation, scenery, photography, journal, radio, night sky.
3. **Mechanical honesty:** condition-driven physics, wear, fuel, altitude, breakdowns, repair.
4. **Living world:** regions, seasons, weather, traffic with personality, roadside stories.
5. **Photography:** a physical camera model, assisted through manual; photos are the story's currency and the player's keepsakes.
6. **Together (designed in, built last):** shared world, convoys, player-run companies.

## World
- Real-world locations, built as separate biomes one at a time: the **Mono Basin** around Lee Vining (Tioga Pass, Mono Lake, Bodie) first, then US-395 south through **Owens Valley and Death Valley**, then **Ladakh and the rest of India** (the grandfather's southern Indian home is the emotional destination), **Iceland**, **the Alps**. Order is flexible; see `docs/MASTER_BLUEPRINT.md` for the tiers.
- Regions are landmasses linked by **sea routes**. Ferries and cargo ships carry the player and vehicle between them (Stage A early; captained and owned ships much later).
- Real-time travel by default: distance has weight. Long-leg options (cruise assist, AI driver for a leg, rest stops) always have a cost and never feel like a free skip.
- Fictional town names and buildings inside real landscapes; fictional vehicle makes; fictional equivalents of real organisations and sign text.

## Vehicles, in story order
Heirloom car (1985-1990 RWD sedan/coupe, rusty to restored) and a light work pickup, then bigger rigid trucks, then articulated trucks with trailers, then more cars and barn finds, then motorcycles. Free Mode unlocks everything that exists. Each vehicle class ships when the story and the engineering are ready.

## Photography
Physical camera model (focal length, aperture, shutter, ISO, depth of field, handheld vs tripod) with Assisted, Semi-manual and Manual modes. Commissions and story beats check measurable criteria (subject in frame, sharp, exposure in range, light, season, weather); the game never claims to judge beauty. Journal, scrapbook and trip posters are the output.

## Realism dial
Per-system sliders from friendly to full simulation (grip, damage, fuel, fatigue, assists), so a huge world can serve many kinds of players.

## Not now
Aircraft, player-captained ships, full mechanic simulation, drones, rigid-body collisions between networked players, real vehicle brands.

## Success metrics
- Phase 2 gate: the director would happily drive the heirloom for five minutes with no objective, and trusts it.
- Vertical slice: the director drives 30 minutes in the Mono Basin with no objective and does not want to stop.
- Career slice: the prologue and first repair loop make the director feel something.

## Source documents
`docs/MASTER_BLUEPRINT.md` (canonical world, narrative and tiers), `docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md`, `docs/CREATIVE_SYSTEMS_COMPENDIUM.md`, `docs/ideas.md` (brainstorm history and decisions), `docs/roadmap.md`, `docs/orchestration.md`, `docs/adr/`.
