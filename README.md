openStarStack
=============
<pre>
Usage: openStarStack [OPTION...] FILE1 FILE2 [FILE3...]
openStarStack - open source astronomy image alignment and stacking software

 The following options are for the alignment process:
  -a, --angle-metric=FLOAT   Angle delta metric - allowed delta between two
                             possible triangle matches angles. (Default=0.002)
  -g, --min-angle=FLOAT      Minimum angle size - min angle size of all three
                             triangle angles, straight lines don't make for
                             good features to match across images (Default=0.1)
                            
  -l, --align-out            Outputs the images of the alignement process,
                             drawing a triangle on the primary image and image
                             of focus
  -m, --rss-metric=FLOAT     Angle RSS metric value - one of a few metrics used
                             to match triangulated stars between the images.
                             (Default=0.001)
  -n, --align-tab=FILE       Save the alignment to file, this is a csv file and
                             is saved by default to ./tempAlign.csv
  -p, --primary-img=FILE     Primary image - the image of interest to align all
                             other images to, if not specified the first image
                             argument will be choosen as the primary
  -s, --num-stars=INT        Number of stars to use - The number of stars to
                             use to form triangles with to try to make a valid
                             match, higher the number, better the accuracy and
                             longer the wait (Default=15,Max=50)
  -t, --threshold=INT        Image threshold value - the minimum DN pixel value
                             of a star required in order for it to be
                             registered as a star (Default=100,Max=255)

 The following options are for the stacking process:
  -0, --zero-sec=FILE        Zero second exposure frame to subtract from each
                             image, dark frame, and flat field.
  -d, --dark-frames=FILE     Dark frames to subtract from each image and flat
                             field, for multiple list as -d FILE1 -d FILE2 -d
                             FILE3 etc.
  -f, --flat-field=FILE      Flat field frame to correct each image, for
                             multiple list as -f FILE1 -f FILE2 -f FILE3 etc.
  -o, --stack-out=FILE       Save the combined final image to disk

  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Enjoy...

Report bugs to <brandonmetz@gmail.com>
</pre>