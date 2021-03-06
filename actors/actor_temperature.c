/*
 * Copyright (c) 2016 Ericsson AB
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>
#include <string.h>
#include "actor_temperature.h"
#include "../fifo.h"
#include "../token.h"
#include "../platform.h"
#include "../port.h"

result_t actor_temperature_fire(struct actor_t *actor)
{
	token_t *in_token = NULL, out_token;
	double temperature = 0.5;
	uint32_t in_data = 0;
	port_t *inport = NULL, *outport = NULL;
	bool did_fire = false;

	inport = port_get_from_name(actor, "measure", PORT_DIRECTION_IN);
	if (inport == NULL) {
		log_error("No port with name 'measure'");
		return FAIL;
	}

	outport = port_get_from_name(actor, "centigrade", PORT_DIRECTION_OUT);
	if (outport == NULL) {
		log_error("No port with name 'centigrade'");
		return FAIL;
	}

	if (fifo_tokens_available(&inport->fifo, 1) == 1 && fifo_slots_available(&outport->fifo, 1) == 1) {
		if (platform_get_temperature(&temperature) != SUCCESS) {
			log_error("Failed to get temperature");
			return FAIL;
		}

		in_token = fifo_peek(&inport->fifo);
		if (token_decode_uint(*in_token, &in_data) != SUCCESS) {
			fifo_cancel_commit(&inport->fifo);
			return FAIL;
		}

		token_set_double(&out_token, temperature);

		if (fifo_write(&outport->fifo, out_token.value, out_token.size) != SUCCESS) {
			log_error("Failed to write token");
			fifo_cancel_commit(&inport->fifo);
			return FAIL;
		}

		fifo_commit_read(&inport->fifo);
		did_fire = true;
	}

	if (did_fire)
		return SUCCESS;

	return FAIL;
}
