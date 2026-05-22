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
|   | Backface culling (extra)  | V |
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
| 9 | Bollen en cylinders       | V |
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
- **UV/normaal input en cube mapping** (sessie 9 deeltaken) zijn niet
  geïmplementeerd.

## Niet-gequoteerde functionaliteit

- Intro-opgave: `IntroColorRectangle` werkt.

## Extra functionaliteit

- `Wireframe` ondersteunt ook `LineDrawing`-figuren (sessie 2 datastructuur
  ook bruikbaar in latere sessies).
- `General.type = "ZBuffering"` en `"LightedZBuffering"` worden beide
  geaccepteerd voor het driehoek-zbuffer-pad.
- Backface culling in het `ZBuffering`-pad (cursus §15). Standaard aan,
  uit te schakelen met `General.backfaceCulling = FALSE` in de .ini.
- Texture mapping ondersteunt zowel ongetextureerde als getextureerde
  figuren via het optionele `texture =`/`uv =`-paar in de Figure-sectie.
- `Thick*` figuren (sessie 9 hoofdtaak) ondersteunen alle base-types:
  `ThickCube`, `ThickTetrahedron`, `ThickOctahedron`, `ThickIcosahedron`,
  `ThickDodecahedron`, `ThickBuckyBall`, `ThickLineDrawing`, `Thick3DLSystem`.


## Afwijkingen van de cursus-algoritmes

Onderstaande implementaties leveren hetzelfde resultaat als de cursus
voorschrijft, maar gebruiken een andere techniek dan in de cursus
beschreven staat. Dit is bewust gedaan en hieronder gedocumenteerd.

### Sessie 5 — Inkleuren van een driehoek (cursus §12)

De cursus beschrijft een scanline-algoritme dat per `y`-waarde de snijpunten
met de driehoekzijden `AB`, `AC` en `BC` berekent om `xL` en `xR` te bepalen.
Deze engine gebruikt in plaats daarvan **barycentrische coördinaten** binnen
de bounding box van de geprojecteerde driehoek: elke pixel binnen de box
wordt getest met
`α = ((bx-x)(cy-y) - (by-y)(cx-x)) / area` enz., en behoort tot de driehoek
als `α, β, γ ≥ 0`. De `1/z`-berekening (cursus p. 43) en `dzdx`/`dzdy`
(p. 44) zijn ongewijzigd. Voor gesloten figuren produceert dit identiek
resultaat; voor zeer dunne driehoeken doet de scanline-methode minder werk.

### Sessie 8 — Texture mapping voor vlakke oppervlakken (cursus §23)

De cursus beschrijft een parametrische aanpak met drie vectoren
`(p, a, b)` die per oppervlak de rechthoek-omhullende vastleggen; de
`(u, v)` van een 3D-punt wordt geanalytisch teruggerekend via een 2×2
deeldeterminant. Deze engine gebruikt in plaats daarvan **per-vertex UV
mapping**: elk hoekpunt van een face krijgt expliciet een `(u, v)` mee
(via het `uv =`-veld), en per pixel wordt perspectief-correct geïnterpoleerd
(cursus §11):
`u = (α·uA/zA + β·uB/zB + γ·uC/zC) / (α/zA + β/zB + γ/zC)`.
Voor vlakke oppervlakken (waar de cursus de techniek voor beschrijft)
levert dit hetzelfde resultaat zolang de hoekpunten consistent geannoteerd
zijn.

### Sessie 6 — BuckyBall (cursus §17, p. 55)

De cursus beschrijft hoe een BuckyBall (truncated icosahedron) wordt
geconstrueerd door elke driehoek van een icosahedron op te delen in een
gelijkzijdige zeshoek en drie hoekdriehoeken. Deze engine retourneert in
plaats daarvan een **gesubdivideerde icosahedron** (`createSphere(1.0, 1)`)
als placeholder. Het resultaat ziet er bolvormig uit maar mist de zeshoeken
en vijfhoeken die een echte BuckyBall karakteriseren. `FractalBuckyBall`
en `ThickBuckyBall` werken correct op deze placeholder.

### Sessie 5 — Triangulatie

De cursus laat de keuze open; deze engine gebruikt **fan-triangulatie**
vanaf hoekpunt 0: een vlak met `n` punten wordt onderverdeeld in `n - 2`
driehoeken `(p₀, pᵢ, pᵢ₊₁)`. Dit werkt correct voor de convexe vlakken van
alle gegenereerde 3D lichamen, maar is niet algemeen voor willekeurige
concave veelhoeken.

### Sessie 9 — Cilinderkappen bij Thick* figuren

De cursus-tip vermeldt: *"Genereer de boven- en ondervlakken van je
cilinders niet. Deze zijn volledig omschreven door de bollen en dus niet
zichtbaar, op mogelijke artefacten na."* Deze engine **behoudt** de
cilinderkappen omdat `createCylinder` ze sowieso al genereert. Bij grote
`radius` t.o.v. de afstand tussen vertices kan dit z-fighting veroorzaken
op de overgang tussen kapje en bol.
