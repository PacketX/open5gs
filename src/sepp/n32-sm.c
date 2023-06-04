/*
 * Copyright (C) 2019-2023 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sbi-path.h"

void sepp_n32_fsm_init(sepp_node_t *node)
{
    sepp_event_t e;

    ogs_assert(node);

    memset(&e, 0, sizeof(e));
    e.node = node;

    ogs_fsm_init(&node->sm, sepp_n32_state_initial, sepp_n32_state_final, &e);
}

void sepp_n32_fsm_fini(sepp_node_t *node)
{
    sepp_event_t e;

    ogs_assert(node);

    memset(&e, 0, sizeof(e));
    e.node = node;

    ogs_fsm_fini(&node->sm, &e);
}

void sepp_n32_state_initial(ogs_fsm_t *s, sepp_event_t *e)
{
    sepp_node_t *node = NULL;

    ogs_assert(s);
    ogs_assert(e);

    node = e->node;
    ogs_assert(node);

    node->t_establish_interval = ogs_timer_add(ogs_app()->timer_mgr,
            sepp_timer_peer_establish, node);
    ogs_assert(node->t_establish_interval);

    OGS_FSM_TRAN(s, &sepp_n32_state_handshake);
}

void sepp_n32_state_final(ogs_fsm_t *s, sepp_event_t *e)
{
    sepp_node_t *node = NULL;

    ogs_assert(s);
    ogs_assert(e);

    sepp_sm_debug(e);

    node = e->node;
    ogs_assert(node);

    ogs_timer_delete(node->t_establish_interval);
}

void sepp_n32_state_handshake(ogs_fsm_t *s, sepp_event_t *e)
{
    sepp_node_t *node = NULL;
    ogs_sbi_message_t *message = NULL;

    ogs_assert(s);
    ogs_assert(e);

    sepp_sm_debug(e);

    node = e->node;
    ogs_assert(node);

    switch (e->h.id) {
    case OGS_FSM_ENTRY_SIG:
        ogs_timer_start(node->t_establish_interval,
            ogs_app()->time.message.sbi.reconnect_interval);

#if 0
        ogs_assert(true == ogs_nnrf_nfm_send_nf_register(node));
#endif
        break;

    case OGS_FSM_EXIT_SIG:
        ogs_timer_stop(node->t_establish_interval);
        break;

#if 0
    case OGS_EVENT_SBI_CLIENT:
        message = e->h.sbi.message;
        ogs_assert(message);

        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NNRF_NFM)

            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)

                if (message->res_status == OGS_SBI_HTTP_STATUS_OK ||
                    message->res_status == OGS_SBI_HTTP_STATUS_CREATED) {
                    ogs_nnrf_nfm_handle_nf_register(node, message);
                    OGS_FSM_TRAN(s, &ogs_sbi_nf_state_registered);
                } else {
                    ogs_error("[%s] HTTP Response Status Code [%d]",
                            NF_INSTANCE_ID(ogs_sbi_self()->node),
                            message->res_status);
                    OGS_FSM_TRAN(s, &ogs_sbi_nf_state_exception);
                }
                break;

            DEFAULT
                ogs_error("[%s] Invalid resource name [%s]",
                        NF_INSTANCE_ID(ogs_sbi_self()->node),
                        message->h.resource.component[0]);
            END
            break;

        DEFAULT
            ogs_error("[%s] Invalid API name [%s]",
                    NF_INSTANCE_ID(ogs_sbi_self()->node),
                    message->h.service.name);
        END
        break;
#endif

    case OGS_EVENT_SBI_TIMER:
        switch(e->h.timer_id) {
        case SEPP_TIMER_PEER_ESTABLISH:
#if 0
            ogs_warn("[%s] Retry to registration with NRF",
                    NF_INSTANCE_ID(ogs_sbi_self()->node));
#else
            ogs_error("Retry to registration with NRF");
#endif

            ogs_timer_start(node->t_establish_interval,
                ogs_app()->time.message.sbi.reconnect_interval);

#if 0
            ogs_assert(true == ogs_nnrf_nfm_send_nf_register(node));
#endif
            break;

        default:
#if 0
            ogs_error("[%s] Unknown timer[%s:%d]",
                    NF_INSTANCE_ID(ogs_sbi_self()->node),
                    ogs_timer_get_name(e->h.timer_id), e->h.timer_id);
#endif
        }
        break;

    default:
        ogs_error("Unknown event %s", sepp_event_get_name(e));
        break;
    }
}

void sepp_n32_state_established(ogs_fsm_t *s, sepp_event_t *e)
{
    sepp_node_t *node = NULL;
    ogs_sbi_message_t *message = NULL;
    ogs_assert(s);
    ogs_assert(e);

    sepp_sm_debug(e);

    node = e->node;
    ogs_assert(node);

    switch (e->h.id) {
    case OGS_FSM_ENTRY_SIG:
#if 0
        ogs_sbi_subscription_spec_t *subscription_spec = NULL;

        ogs_info("[%s] NF registered [Heartbeat:%ds]",
                NF_INSTANCE_ID(ogs_sbi_self()->node),
                node->time.heartbeat_interval);

        if (node->time.heartbeat_interval) {
            ogs_timer_start(node->t_heartbeat_interval,
                ogs_time_from_sec(node->time.heartbeat_interval));
            ogs_timer_start(node->t_no_heartbeat,
                ogs_time_from_sec(
                    node->time.heartbeat_interval +
                    ogs_app()->time.node.no_heartbeat_margin));
        }

        ogs_list_for_each(
            &ogs_sbi_self()->subscription_spec_list, subscription_spec) {
            ogs_nnrf_nfm_send_nf_status_subscribe(
                    ogs_sbi_self()->node->nf_type,
                    ogs_sbi_self()->node->id,
                    subscription_spec->subscr_cond.nf_type,
                    subscription_spec->subscr_cond.service_name);
        }
#endif
        break;

    case OGS_FSM_EXIT_SIG:
#if 0
        ogs_info("[%s] NF de-registered",
                NF_INSTANCE_ID(ogs_sbi_self()->node));

        if (node->time.heartbeat_interval) {
            ogs_timer_stop(node->t_heartbeat_interval);
            ogs_timer_stop(node->t_no_heartbeat);
        }

        if (!OGS_FSM_CHECK(&node->sm, ogs_sbi_nf_state_exception)) {
            ogs_assert(true ==
                    ogs_nnrf_nfm_send_nf_de_register(node));
        }
#endif
        break;

#if 0
    case OGS_EVENT_SBI_CLIENT:
        message = e->h.sbi.message;
        ogs_assert(message);

        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NNRF_NFM)

            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)

                if (message->res_status == OGS_SBI_HTTP_STATUS_NO_CONTENT ||
                    message->res_status == OGS_SBI_HTTP_STATUS_OK) {
                    if (node->time.heartbeat_interval)
                        ogs_timer_start(node->t_no_heartbeat,
                            ogs_time_from_sec(
                                node->time.heartbeat_interval +
                                ogs_app()->time.node.
                                    no_heartbeat_margin));
                } else {
                    ogs_warn("[%s] HTTP response error [%d]",
                            NF_INSTANCE_ID(ogs_sbi_self()->node),
                            message->res_status);
                    OGS_FSM_TRAN(s, &ogs_sbi_nf_state_exception);
                }

                break;

            DEFAULT
                ogs_error("[%s] Invalid resource name [%s]",
                        NF_INSTANCE_ID(ogs_sbi_self()->node),
                        message->h.resource.component[0]);
            END
            break;

        DEFAULT
            ogs_error("[%s] Invalid API name [%s]",
                    NF_INSTANCE_ID(ogs_sbi_self()->node),
                    message->h.service.name);
        END
        break;

    case OGS_EVENT_SBI_TIMER:
        switch(e->h.timer_id) {
        case OGS_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL:
            if (node->time.heartbeat_interval)
                ogs_timer_start(node->t_heartbeat_interval,
                    ogs_time_from_sec(node->time.heartbeat_interval));

            ogs_assert(true == ogs_nnrf_nfm_send_nf_update(node));
            break;

        case OGS_TIMER_NF_INSTANCE_NO_HEARTBEAT:
            ogs_error("[%s:%s] No heartbeat",
                    NF_INSTANCE_ID(ogs_sbi_self()->node),
                    OpenAPI_nf_type_ToString(
                        NF_INSTANCE_TYPE(ogs_sbi_self()->node)));
            OGS_FSM_TRAN(s, &ogs_sbi_nf_state_will_register);
            break;

        case OGS_TIMER_NF_INSTANCE_VALIDITY:
            ogs_assert(!NF_INSTANCE_TYPE_IS_NRF(node));
            ogs_assert(node->id);

            ogs_info("[%s] NF expired", node->id);
            OGS_FSM_TRAN(s, &ogs_sbi_nf_state_de_registered);
            break;

        default:
            ogs_error("[%s:%s] Unknown timer[%s:%d]",
                    OpenAPI_nf_type_ToString(node->nf_type),
                    node->id ? node->id : "Undefined",
                    ogs_timer_get_name(e->h.timer_id), e->h.timer_id);
        }
        break;
#endif

    default:
#if 0
        ogs_error("[%s:%s] Unknown event %s",
                OpenAPI_nf_type_ToString(node->nf_type),
                node->id ? node->id : "Undefined",
                sepp_event_get_name(e));
#endif
        break;
    }
}

void sepp_n32_state_terminated(ogs_fsm_t *s, sepp_event_t *e)
{
    sepp_node_t *node = NULL;
    ogs_assert(s);
    ogs_assert(e);

    sepp_sm_debug(e);

    node = e->node;
    ogs_assert(node);

    switch (e->h.id) {
    case OGS_FSM_ENTRY_SIG:
#if 0
        ogs_info("[%s] NF de-registered",
                NF_INSTANCE_ID(ogs_sbi_self()->node));
#endif
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    default:
#if 0
        ogs_error("[%s:%s] Unknown event %s",
                OpenAPI_nf_type_ToString(node->nf_type),
                node->id ? node->id : "Undefined",
                sepp_event_get_name(e));
#endif
        break;
    }
}

void sepp_n32_state_exception(ogs_fsm_t *s, sepp_event_t *e)
{
    sepp_node_t *node = NULL;
    ogs_sbi_message_t *message = NULL;
    ogs_assert(s);
    ogs_assert(e);

    sepp_sm_debug(e);

    node = e->node;
    ogs_assert(node);

    switch (e->h.id) {
    case OGS_FSM_ENTRY_SIG:
        ogs_timer_start(node->t_establish_interval,
            ogs_app()->time.message.sbi.reconnect_interval_in_exception);
        break;

    case OGS_FSM_EXIT_SIG:
        ogs_timer_stop(node->t_establish_interval);
        break;

#if 0
    case OGS_EVENT_SBI_TIMER:
        switch(e->h.timer_id) {
        case SEPP_TIMER_PEER_ESTABLISH:
            ogs_warn("[%s] Retry to registration with NRF",
                    NF_INSTANCE_ID(ogs_sbi_self()->node));

            OGS_FSM_TRAN(s, &ogs_sbi_nf_state_will_register);
            break;

        default:
            ogs_error("[%s:%s] Unknown timer[%s:%d]",
                    OpenAPI_nf_type_ToString(node->nf_type),
                    node->id ? node->id : "Undefined",
                    ogs_timer_get_name(e->h.timer_id), e->h.timer_id);
        }
        break;

    case OGS_EVENT_SBI_CLIENT:
        message = e->h.sbi.message;
        ogs_assert(message);

        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NNRF_NFM)

            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)
                break;
            DEFAULT
                ogs_error("Invalid resource name [%s]",
                        message->h.resource.component[0]);
            END
            break;
        DEFAULT
            ogs_error("Invalid API name [%s]", message->h.service.name);
        END
        break;
#endif

    default:
#if 0
        ogs_error("[%s:%s] Unknown event %s",
                OpenAPI_nf_type_ToString(node->nf_type),
                node->id ? node->id : "Undefined",
                sepp_event_get_name(e));
#endif
        break;
    }
}
