# 3D-print models

Here you can find the 3D-print ready models of the clock. The files also contain PrusaSlicer/SuperSlicer metadata for optimal printing settings and custom support definitions, therefore I highly recommend to use [SuperSlicer](https://github.com/supermerill/SuperSlicer) to slice the models for your printer (v2.4 or later is required).

These tables below show which material to use for which part and how much of it is necessary:

## Approximate opaque material demand:

| Part                 | Weight* | Pieces |  Total   | Supports |
|----------------------|--------:|-------:|---------:|:--------:|
| [Base](base.3mf)     |     86g |      1 |    86g   |    No    |
| [Bottom](bottom.3mf) |     32g |      1 |    32g   |  Manual  |
| [Colon](colon.3mf)   |      9g |      1 |     9g   |  Manual  |
| [Digit](digit.3mf)   |     40g |      4 |   160g   |  Manual  |
|                      |         |        | **287g** |          |

*\* Excluding skirt/brim, and supports.*

The above weights were measured using my recommendation for an opaque material: [FiberSatin (black) by Fiberology](https://fiberlogy.com/en/fiberlogy-filaments/fibersatin/).

## Approximate translucent material demand:

| Part                       | Weight* | Pieces |   Total   | Supports |
|----------------------------|--------:|-------:|----------:|:--------:|
| [Segment A](segment-a.3mf) |    ~6g  |      4 |    ~24g   |    No    |
| [Segment B](segment-b.3mf) |    ~6g  |      4 |    ~24g   |    No    |
| [Segment C](segment-c.3mf) |    ~6g  |      4 |    ~24g   |    No    |
| [Segment D](segment-d.3mf) |    ~6g  |      4 |    ~24g   |    No    |
| [Segment E](segment-e.3mf) |    ~6g  |      4 |    ~24g   |    No    |
| [Segment F](segment-f.3mf) |    ~6g  |      4 |    ~24g   |    No    |
| [Segment G](segment-g.3mf) |    ~6g  |      4 |    ~24g   |    No    |
| [Dot](dot.3mf)             |    ~1g  |      2 |     ~2g   |    No    |
|                            |         |        | **~170g** |          |

*\* Excluding skirt/brim, and supports.*

The above weights were measured using my recommendation for an translucent material: [PLA Extrafill Natural by Fillamentum](https://fillamentum.com/collections/pla-extrafill-filament/).

## Custom supports

If you can't use the embedded support information, here are some pictures as a reference to help establish the proper supports (seen with green color):

![Support for Bottom](supports-bottom.png)
![Support for Colon](supports-colon.png)
![Support for Digit](supports-digit.png)

## Sequential prints

If you have a large printer you may leverage the benefits of sequential printing even for one clock or to print parts for multiple clocks.

| File                                                | Notes |
|-----------------------------------------------------|-------|
| [Base with Bottom](base-with-bottom-sequential.3mf) | Contains the base and bottom parts two times. Just delete one pair before slicing if you don't need it. |
| [Colon](colon-sequential.3mf)                       | Contains 8 pieces for "mass production". |
| [Digit](digit-sequential.3mf)                       | Contains a set of 4 digits. |
| [Segments](segments-sequential.3mf)                 | Contains 2 sets of the 7 segments and a single dot. Print this twice to get all necessary translucent parts for a single clock. |