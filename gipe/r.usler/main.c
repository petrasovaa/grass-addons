
/****************************************************************************
 *
 * MODULE:       r.usler
 * AUTHOR(S):    Natalia Medvedeva - natmead@gmail.com
 *		 Yann Chemin - ychemin@gmail.com
 * PURPOSE:      Calculates USLE R factor 
 * 		 Rainfall Erosion index according to four methods 
 *
 * COPYRIGHT:    (C) 2002-2006 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *   	    	 License (>=v2). Read the file COPYING that comes with GRASS
 *   	    	 for details.
 * 
 *****************************************************************************/
     
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/glocale.h>
double elswaify_1985(double annaul_pmm);

double morgan_1974(double annual_pmm);

double foster_1981(double annual_pmm);

double roose_1975(double annual_pmm);

int main(int argc, char *argv[]) 
{
    struct Cell_head cellhd;	/*region+header info */

    char *mapset;		/*mapset name */

    int nrows, ncols;

    int row, col;

    char *nameflag;		/*Switch for particular method */

    struct GModule *module;

    struct Option *input1, *input2, *output;

    struct History history;	/*metadata */

    

	/************************************/ 
	/* FMEO Declarations**************** */ 
    char *name;			/*input raster name */

    char *result;		/*output raster name */

    
	/*File Descriptors */ 
    int infd_annual_pmm;

    int outfd;

    char *annual_pmm;

    int i = 0, j = 0;

    void *inrast_annual_pmm;

    DCELL * outrast;
    RASTER_MAP_TYPE data_type_output = DCELL_TYPE;
    RASTER_MAP_TYPE data_type_annual_pmm;
    

	/************************************/ 
	G_gisinit(argv[0]);
    module = G_define_module();
    module->keywords = _("rainfall, erosion, USLE");
    module->description = _("USLE R factor, Rainfall erosivity index.");
    
	/* Define the different options */ 
	input1 = G_define_option();
    input1->key = _("name");
    input1->type = TYPE_STRING;
    input1->required = YES;
    input1->gisprompt = _("Name of method to use");
    input1->description =
	_("Name of USLE R equation: roose, morgan, foster, elswaify.");
    input1->answer = _("morgan");
    input2 = G_define_standard_option(G_OPT_R_INPUT);
    input2->key = _("annual_precip");
    input2->description = _("Name of the annual precipitation map");
    input2->answer = _("annual_pmm");
    output = G_define_standard_option(G_OPT_R_OUTPUT);
    output->key = _("usler");
    output->description = _("Name of the output usler layer");
    output->answer = _("usler");
    

	/********************/ 
	if (G_parser(argc, argv))
	exit(EXIT_FAILURE);
    nameflag = input1->answer;
    annual_pmm = input2->answer;
    result = output->answer;
    

	/***************************************************/ 
	mapset = G_find_cell2(annual_pmm, "");
    if (mapset == NULL) {
	G_fatal_error(_("cell file [%s] not found"), annual_pmm);
    }
    data_type_annual_pmm = G_raster_map_type(annual_pmm, mapset);
    if ((infd_annual_pmm = G_open_cell_old(annual_pmm, mapset)) < 0)
	G_fatal_error(_("Cannot open cell file [%s]"), annual_pmm);
    if (G_get_cellhd(annual_pmm, mapset, &cellhd) < 0)
	G_fatal_error(_("Cannot read file header of [%s])"), annual_pmm);
    inrast_annual_pmm = G_allocate_raster_buf(data_type_annual_pmm);
    

	/***************************************************/ 
	G_debug(3, "number of rows %d", cellhd.rows);
    nrows = G_window_rows();
    ncols = G_window_cols();
    outrast = G_allocate_raster_buf(data_type_output);
    
	/* Create New raster files */ 
	if ((outfd = G_open_raster_new(result, data_type_output)) < 0)
	G_fatal_error(_("Could not open <%s>"), result);
    
	/* Process pixels */ 
	for (row = 0; row < nrows; row++)
	 {
	DCELL d;
	DCELL d_annual_pmm;
	G_percent(row, nrows, 2);
	
	    /* read input map */ 
	    if (G_get_raster_row
		(infd_annual_pmm, inrast_annual_pmm, row,
		 data_type_annual_pmm) < 0)
	    G_fatal_error(_("Could not read from <%s>"), annual_pmm);
	
	    /*process the data */ 
	    for (col = 0; col < ncols; col++)
	     {
	    d_annual_pmm = ((DCELL *) inrast_annual_pmm)[col];
	    if (G_is_d_null_value(&d_annual_pmm)) {
		G_set_d_null_value(&outrast[col], 1);
	    }
	    else {
		

				/************************************/ 
		    /*calculate morgan       */ 
		    if (!strcoll(nameflag, "morgan")) {
		    d = morgan_1974(d_annual_pmm);
		    outrast[col] = d;
		}
		
		    /*calculate roose                   */ 
		    if (!strcoll(nameflag, "roose")) {
		    d = roose_1975(d_annual_pmm);
		    outrast[col] = d;
		}
		
		    /*calculate foster                  */ 
		    if (!strcoll(nameflag, "foster")) {
		    d = foster_1981(d_annual_pmm);
		    outrast[col] = d;
		}
		
		    /*calculate elswaify                */ 
		    if (!strcoll(nameflag, "elswaify")) {
		    d = elswaify_1985(d_annual_pmm);
		    outrast[col] = d;
		}
	    }
	    }
	if (G_put_raster_row(outfd, outrast, data_type_output) < 0)
	    G_fatal_error(_("Cannot write to output raster file"));
	}
    G_free(inrast_annual_pmm);
    G_close_cell(infd_annual_pmm);
    G_free(outrast);
    G_close_cell(outfd);
    G_short_history(result, "raster", &history);
    G_command_history(&history);
    G_write_history(result, &history);
    exit(EXIT_SUCCESS);
}


