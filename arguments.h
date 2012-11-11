#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

/* Program documentation. */
static char doc[] =
 "openStack - open source astronomy image alignment and stacking software\
\vEnjoy...";

/* A description of the arguments we accept. */
static char args_doc[] = "FILE1 FILE2 [FILE3...]";

/* Keys for options without short-options. */
#define ALIGN_OUT 1

/* The options we understand. */
static struct argp_option options[] = {
	{0,0,0,0, 								 "The following options are for the alignment process:" },
	{"threshold",  	't', "INT", 0, "Image threshold value - the minimum DN pixel value of a star required in order for it to be registered as a star (Default=100,Max=255)" },
	{"num-stars",	's', "INT", 0, "Number of stars to use - The number of stars to use to form triangles with to try to make a valid match, (Default=15,Max=50)" },
	{"rss-metric",	'm', "FLOAT", 0, "Angle RSS metric value - one of a few metrics used to match triangulated stars between the images. (Default=0.001)" },
	{"angle-metric",'a', "FLOAT", 0, "Angle delta metric - allowed delta between two possible triangle matches angles. (Default=0.002)"},
	{"min-angle",		'g', "FLOAT", 0, "Minimum angle size - min angle size of all three triangle angles, straight lines don't make for good features to match across images (Default=0.1)" },
	{"primary-img",	'p', "FILE", 0, "Primary image - the image of interest to align all other images to, if not specified the first image argument will be choosen as the primary" },
	{"align-tab",		'n', "FILE", 0, "Save the alignment to file, this is a csv file and is saved by default to ./tempAlign.csv" },
	{"align-out",		'l', 0, 0, "Outputs the images of the alignement process, drawing a triangle on the primary image and image of focus" },  
	{0,0,0,0, 									"The following options are for the stacking process:" },
	{"stack-out",	'o', "FILE", 0, "Save the combined final image to disk" },
	{"dark-frames",	'd', "FILE", 0, "Dark frames to subtract from each image and flat field, for multiple list as -d FILE1 -d FILE2 -d FILE3 etc." },
	{"flat-field",	'f', "FILE", 0, "Flat field frame to correct each image, for multiple list as -f FILE1 -f FILE2 -f FILE3 etc." },
	{"zero-sec",	'0', "FILE", 0, "Zero second exposure frame to subtract from each image, dark frame, and flat field." },
{ 0 }
};

struct arguments {
	char **file;             					//image files
	unsigned int	numFiles;					//number of image files
	unsigned char threshold;					//-t
	float rssMetric, angleMetric, minAngle; 	//-m, -a, -g options
	char *primaryImage;							//-p
	char *imageOutFilename;						//-p
	char *darkFramesFilename[255];						//-d
	unsigned char darkFramesIndex;						//-d
	char *flatFieldFilename[255];						//-f
	unsigned char flatFieldIndex;						//-f
	char  alignOut;							
	char *alignmentFilename;						//-n
	unsigned int triNStars;
	char *zeroPixFilename;
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state){
	/* Get the input argument from argp_parse, which we
			know is a pointer to our arguments structure. */
 struct arguments *arguments = state->input;

	switch (key)	{
//Aligning arguments
		case 't':
			arguments->threshold = arg ? atoi(arg) : 75;
			break;
		case 'm':
			arguments->rssMetric = arg ? atof(arg) : 0.001;
			break;
		case 'a':
			arguments->angleMetric = arg ? atof(arg) : 0.002;
			break;
		case 'g':
			arguments->minAngle = arg ? atof(arg) : 0.1;
			break;
		case 'p':
			arguments->primaryImage = arg ? arg : '\0';
			break;
		case 'n':
			arguments->alignmentFilename = arg ? arg : '\0';
			break;
		case 'l':
			arguments->alignOut = 1;
			break;
		case 's':
			arguments->triNStars = arg ? atoi(arg) : 30;
			break;
//Stacking arguments	
		case 'f':
			arguments->flatFieldFilename[arguments->flatFieldIndex] = arg ? arg : '\0';
			arguments->flatFieldIndex++;
			break;
		case 'd':
			arguments->darkFramesFilename[arguments->darkFramesIndex] = arg ? arg : '\0';
			arguments->darkFramesIndex++;
			break;
		case 'o':
			arguments->imageOutFilename = arg ? arg : '\0';
			break; 
		case '0':
			arguments->zeroPixFilename = arg ? arg : '\0';
			break; 
		case ARGP_KEY_ARGS:
			arguments->file = &state->argv[state->next];
			arguments->numFiles = state->argc - state->next;	
		break;

		case ARGP_KEY_END:
			if (arguments->numFiles < 2) 
					argp_usage(state);
			break;

	 default:
		 return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };


#endif
