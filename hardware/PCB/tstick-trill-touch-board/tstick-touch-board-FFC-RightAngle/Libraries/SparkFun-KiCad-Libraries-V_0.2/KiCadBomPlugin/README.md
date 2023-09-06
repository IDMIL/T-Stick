BOM tool
====================================

This folder contains a single file, bom2groupedCsv.xsl.  It can be run through tools->Generate Bill of Materials

Use the command line: 
xsltproc -o "%O.csv" "C:\github\SparkFun-KiCad-Libraries\Conversion\KiCadBomPlugin\KicadBOM\bom2groupedCsv.xsl" "%I"

The information originally came from a comment by fritsjan, referenced by ContextualElectronics in a video:
https://www.youtube.com/watch?v=9yFZ6PUwKL8

The file came from this zip:

[KicadBom.zip](https://kicad-info.s3.amazonaws.com/3929625ac5cd971344cc17cda0c3877a3981d489907.zip)

You don't need to install the zip contents as described in the video, just execute the .xsl file with the changed command line.  I suspect xsltproc wasn't distribued with earlier versions of KiCad.