/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 * (C) Copyright 2015-2018 Hewlett Packard Enterprise Development LP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett Packard Enterprise, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *      Bhaskara Bhatt <bhaskara.hg@hp.com>
 */

#include "hpi_test.h"

int main(int argc, char **argv)
{
        int number_resources=0, areaTypeOption;
        SaErrorT rv;
        SaHpiSessionIdT sessionid;
        SaHpiResourceIdT resourceid;
        SaHpiIdrIdT IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        SaHpiIdrAreaTypeT areaType = SAHPI_IDR_AREATYPE_CHASSIS_INFO;
        SaHpiEntryIdT area_id;
        SaHpiResourceIdT resourceid_list[RESOURCE_CAP_LENGTH] = {0};
        SaHpiCapabilitiesT capability = SAHPI_CAPABILITY_INVENTORY_DATA;

        printf("saHpiIdrAreaAddById : Test for hpi IDR area add by id"
               " function\n");

        rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sessionid, NULL);
        if (rv != SA_OK)
        {
                printf("saHpiSessionOpen failed with error: %s\n",
                       oh_lookup_error(rv));
                return rv;
        }

        /* Discover the resources with IDR capability */
        printf("\nListing the resource with IDR capability \n");
        rv = discover_resources(sessionid, capability, resourceid_list,
                                &number_resources);
        if (rv != SA_OK)
        {
                exit(-1);
        }

        printf("\nPlease enter the resource id: ");
        scanf("%d", &resourceid);

        printf("\nPossible AreaType options are :");
        printf("\n\tPress 1 for INTERNAL USE");
        printf("\n\tPress 2 for CHASSIS INFO");
        printf("\n\tPress 3 for BOARD INFO");
        printf("\n\tPress 4 for PRODUCT INFO");
        printf("\n\tPress 5 for OEM");
        printf("\nPlease enter your option for areaType: ");
        scanf("%d", &areaTypeOption);

        if(areaTypeOption == 1)
                areaType=SAHPI_IDR_AREATYPE_INTERNAL_USE;
        else if (areaTypeOption == 2)
                areaType=SAHPI_IDR_AREATYPE_CHASSIS_INFO;
        else if (areaTypeOption == 3)
                areaType=SAHPI_IDR_AREATYPE_BOARD_INFO;
        else if (areaTypeOption == 4)
                areaType=SAHPI_IDR_AREATYPE_PRODUCT_INFO;
        else if (areaTypeOption == 5)
                areaType=SAHPI_IDR_AREATYPE_OEM;
        else
        {
                printf("Wrong option entered. Exiting");
                exit (-1);
        }
	printf("\nPlease enter Area id: ");
	scanf("%d", &area_id);

        rv = saHpiIdrAreaAddById(sessionid, resourceid, IdrId, areaType,
                                 area_id);
        if (rv != SA_OK)
        {
                printf("saHpiIdrAreaAdd failed with error: %s\n",
                       oh_lookup_error(rv));
                printf("Test case - FAIL\n");
        }
        else
        {
                printf("Test case - PASS\n");
        }

        rv = saHpiSessionClose(sessionid);
        return 0;
}
