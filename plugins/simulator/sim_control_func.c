/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Christina Hernandez <hernanc@us.ibm.com>
 */

#include <sim_init.h>

/**
 * sim_get_control_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @cid: Control ID.
 * @mode: Location to store control's operational mode.
 * @state: Location to store control's state.
 *
 * Retrieves a control's operational mode and/or state. Both @mode and @state
 * may be NULL (e.g. check for presence).
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL.
 * SA_ERR_HPI_INVALID_CMD - Control is write-only.
 * SA_ERR_HPI_INVALID_DATA - @state contain invalid text line number.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_NOT_PRESENT - Control doesn't exist.
 **/
SaErrorT sim_get_control_state(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiCtrlNumT cid,
				   SaHpiCtrlModeT *mode,
				   SaHpiCtrlStateT *state)
{
	SaErrorT err;
	SaHpiCtrlStateT working_state;
        struct ControlInfo *cinfo;
	struct sim_value get_value;
	
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
	if (!hnd) {
		dbg("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
        struct sim_hnd *custom_handle = (struct sim_hnd *)handle->data;
	if (!custom_handle) {
		dbg("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	sim_lock_handler(custom_handle);
	memset(&working_state, 0, sizeof(SaHpiCtrlStateT));

	/* Check if resource exists and has control capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Find control and its mapping data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
	if (rdr == NULL) {
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	cinfo = (struct ControlInfo *)oh_get_rdr_data(handle->rptcache, cid, rdr->RecordId);
 	if (cinfo == NULL) {
		dbg("No control data. Control=%s", rdr->IdString.Data);
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       

	if (rdr->RdrTypeUnion.CtrlRec.WriteOnly) {
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_CMD);
	}
	if (!mode && !state) {
		sim_unlock_handler(custom_handle);
		return(SA_OK);
	}
	
	if (state) {
		if (state->Type == SAHPI_CTRL_TYPE_TEXT) {
			if (state->StateUnion.Text.Line != SAHPI_TLN_ALL_LINES ||
			    state->StateUnion.Text.Line > rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxLines) {
			    	sim_unlock_handler(custom_handle);
				return(SA_ERR_HPI_INVALID_DATA);
			}
		}

		/* Find control's state */
		working_state.Type = rdr->RdrTypeUnion.CtrlRec.Type;
		
		switch (rdr->RdrTypeUnion.CtrlRec.Type) {
		case SAHPI_CTRL_TYPE_DIGITAL:
			sim_unlock_handler(custom_handle);
			dbg("Digital controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_DISCRETE:
			working_state.StateUnion.Discrete = get_value.integer;
			break;
		case SAHPI_CTRL_TYPE_ANALOG:
			sim_unlock_handler(custom_handle);
			dbg("Analog controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_STREAM:
			sim_unlock_handler(custom_handle);
			dbg("Stream controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_TEXT:
			sim_unlock_handler(custom_handle);
			dbg("Text controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_OEM:
			sim_unlock_handler(custom_handle);
			dbg("Oem controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		default:
			dbg("%s has invalid control state=%d.", cinfo->mib.oid, working_state.Type);
			sim_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	if (state) memcpy(state, &working_state, sizeof(SaHpiCtrlStateT));
	if (mode) *mode = cinfo->cur_mode;
	
	sim_unlock_handler(custom_handle);
	return(SA_OK);
}

/**
 * sim_set_control_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @cid: Control ID.
 * @mode: Control's operational mode to set.
 * @state: Pointer to control's state to set.
 *
 * Sets a control's operational mode and/or state. Both @mode and @state
 * may be NULL (e.g. check for presence).
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL.
 * SA_ERR_HPI_INVALID_DATA - @state contains bad text control data.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_NOT_PRESENT - Control doesn't exist.
 **/
SaErrorT sim_set_control_state(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiCtrlNumT cid,
				   SaHpiCtrlModeT mode,
				   SaHpiCtrlStateT *state)
{
	SaErrorT err;
        struct ControlInfo *cinfo;
	struct sim_value set_value;

        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
	if (!hnd) {
		dbg("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}	
	
        struct sim_hnd *custom_handle = (struct sim_hnd *)handle->data;
	if (!custom_handle) {
		dbg("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	sim_lock_handler(custom_handle);
	/* Check if resource exists and has control capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Find control and its mapping data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
	if (rdr == NULL) {
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}

	/*Note: cinfo must be changed to write to David A's API, not the rptcache*/	
	cinfo = (struct ControlInfo *)oh_get_rdr_data(handle->rptcache, cid, rdr->RecordId);
 	if (cinfo == NULL) {
		dbg("No control data. Control=%s", rdr->IdString.Data);
		sim_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       

	/* Validate static control state and mode data */
	err = oh_valid_ctrl_state_mode(&(rdr->RdrTypeUnion.CtrlRec), mode, state);
	if (err) {
		sim_unlock_handler(custom_handle);
		return(err);
	}

	/* Write control state */
	if (mode != SAHPI_CTRL_MODE_AUTO && state) {
		switch (state->Type) {
		case SAHPI_CTRL_TYPE_DIGITAL:
			dbg("Digital controls not supported.");
			sim_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_DISCRETE:
			set_value.type = ASN_INTEGER;
			set_value.str_len = 1;
			set_value.integer = state->StateUnion.Discrete;
			break;
		case SAHPI_CTRL_TYPE_ANALOG:
			dbg("Analog controls not supported.");
			sim_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_STREAM:
			dbg("Stream controls not supported.");
			sim_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_TEXT:
			dbg("Text controls not supported.");
			sim_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_OEM:	
			dbg("OEM controls not supported.");
			sim_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		default:
			dbg("Invalid control state=%d", state->Type);
			sim_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	/* Write control mode, if changed */
	/*Change to write to David A's API, not the rptcache*/
	if (mode != cinfo->cur_mode) {
		cinfo->cur_mode = mode;
	}
	
	sim_unlock_handler(custom_handle);
        return(SA_OK);
}