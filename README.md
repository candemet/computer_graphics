## Gequoteerde functionaliteit

V: Werkend  
-: Deels werkend met gekende problemen (onderaan beschreven)  
X: Niet werkend of niet geimplementeerd
(blanco): TODO


|   | Functionaliteit      | Status |
|---|---------------------------|---|
| 1 | 2D L-systemen             | V |
|   | Met haakjes               | V |
|   | Stochastisch              | X |
| 2 | Transformaties            | V |
|   | Eye-point                 | V |
|   | Projectie                 | V |
| 3 | Platonische Lichamen      | V |
|   | Kegel en cylinder         | V |
|   | Bol                       | V |
|   | Torus                     | V |
|   | 3D L-systemen             | V |
| 4 | Z-buffering (lijnen)      | V |
| 5 | Triangulatie              | V |
|   | Z-buffering (driehoeken)  | V |
| 6 | 3D fractalen              | X |
|   | BuckyBall                 | X |
|   | Mengerspons               | X |
|   | View Frustum              | X |
| 7 | Ambient licht             | X |
|   | Diffuus licht (oneindig)  | X |
|   | Diffuus licht (puntbron)  | X |
|   | Speculair licht           | X |
| 8 | Schaduw                   | X |
|   | Texture mapping           | X |
| 9 | Bollen en cylinders       | X |
|   | UV-coordinaten            | X |
|   | Cube mapping              | X |

Geimplementeerde vorm van texture mapping: n.v.t.

## Gekende problemen
- Voor 2D L-systemen gebruikt de engine fallback-zoekpaden met oude mapnamen (`l_systems/`, `lsystem2D/`). In de huidige structuur staan deze bestanden in `data/l_systems/`.
- Resolutie van bmp files matchen niet met png files
- Session 5 triangulatie gebruikt fan-triangulatie op vlakken met 3 of meer punten; dit is voldoende voor de gekende 3D lichamen, maar niet bedoeld als algemene triangulatie voor willekeurige concave polygonen.
- Backface culling is actief in `ZBuffering`, maar de gevraagde runtime-metingen/rapportering voor de extra oefening zijn nog niet toegevoegd.
Nog niet alles is minstens 80%: 3DLichamen en ZBufferingLijnen

## Niet-gequoteerde functionaliteit
- Intro-opgaves: `IntroColorRectangle` werkt.

## Extra functionaliteit, niet in de opgaves beschreven
- Ondersteuning voor `Wireframe` met `LineDrawing`-figuren (sessie 2) is toegevoegd.
- Backface culling in de `ZBuffering`-renderpad is aanwezig.
- Ondersteuning voor `General.type = ZBuffering` is toegevoegd (triangles + z-buffer).
