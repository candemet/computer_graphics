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
| 6 | 3D fractalen              | V |
|   | BuckyBall                 | - |
|   | Mengerspons               | V |
|   | View Frustum              | X |
| 7 | Ambient licht             | V |
|   | Diffuus licht (oneindig)  | V |
|   | Diffuus licht (puntbron)  | V |
|   | Speculair licht           | V |
| 8 | Schaduw                   | V |
|   | Texture mapping           | V |
| 9 | Bollen en cylinders       | X |
|   | UV-coordinaten            | X |
|   | Cube mapping              | X |

## Gekende problemen

- **Stochastische L-systemen** zijn niet geïmplementeerd; de bijhorende
  uitbreiding van het `.L2D`-bestandsformaat en `l_parser` is niet gedaan.
- **BuckyBall** retourneert momenteel een gesubdivideerde icosahedron
  (een "icosphere"), niet de truncated icosahedron uit de cursus.
  `FractalBuckyBall` werkt op deze placeholder.
- **Resolutie van .bmp output** komt soms 1 pixel af van de referentie .png
  door verschil tussen `lround` en `ceil` in de twee renderpaden
  (`draw2DLines` versus `computeProjectionParamters`).
- **Triangulatie** gebruikt fan-triangulatie: correct voor de convexe vlakken
  van de gekende 3D lichamen, niet algemeen voor willekeurige concave veelhoeken.
- **2D L-systemen** gebruikt fallback-zoekpaden voor oude mapnamen
  (`l_systems/`, `lsystem2D/`); in de huidige structuur staan deze bestanden
  in `data/l_systems/`.
- **Lijnen die de cameraplane doorkruisen** (één endpoint met z ≥ 0) worden
  in de `Wireframe`/`ZBufferedWireframe` modi weggelaten in plaats van
  geclipped. Dit voorkomt projectie-artefacten maar laat in pathologische
  scènes (oog binnenin een figuur) lijnen verdwijnen.
- **Schaduwen** worden enkel ondersteund voor puntbronnen (zoals de cursus
  vereist, p. 67); diffuse oneindig-bronnen werpen geen schaduw.

## Niet-gequoteerde functionaliteit

- Intro-opgave: `IntroColorRectangle` werkt.

## Extra functionaliteit

- `Wireframe` ondersteunt ook `LineDrawing`-figuren (sessie 2 datastructuur
  ook bruikbaar in latere sessies).
- `General.type = "ZBuffering"` en `"LightedZBuffering"` worden beide
  geaccepteerd voor het driehoek-zbuffer-pad.
- Backface culling in het `ZBuffering`-pad (cursus §15). Standaard aan,
  uit te schakelen met `General.backfaceCulling = FALSE` in de .ini.
  De runtime-metingen voor de extra oefening van sessie 5 zijn niet toegevoegd.
- Texture mapping ondersteunt zowel ongetextureerde als getextureerde
  figuren via het optionele `texture =`/`uv =`-paar in de Figure-sectie.