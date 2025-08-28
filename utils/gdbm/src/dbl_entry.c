#include "pqlx_errors.h"
#include "dbl_iface.h"
#include "dblinc.h"

int dblg_request(DBLDI *dbldi)
{
	int ret = FAILURE;
	switch(dbldi->descr.tableData)
	{
		case DEFAULTST:			// process DEFAULTS Table request
			ret = dblg_defaults(dbldi);
		break;

		case FILTERST:			// process FILTER Table request
			ret = dblg_filter(dbldi);
		break;

#ifndef PQL_ONLY
		case PDFST:				// process PDF Table request
//			ret = dblg_pdf(dbldi);
		break;

		case PDFSRVRST:			// process PDFSRVR Table request
			ret = dblg_pdfsrvr(dbldi);
		break;

		case PDFDFAULTST:
			ret = dblg_pdf(dbldi);
		break;
				
		case STNCHNFLTRST:
		case STNDFAULTST:
		case STNCHNYSCALE:
		case STNGRPYSCALE:
			ret = dblg_STN(dbldi);
		break;
#endif
	}

	return ret;
}
