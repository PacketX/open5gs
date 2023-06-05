/*
 * Copyright (C) 2023 by Sukchan Lee <acetcom@gmail.com>
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

#include "n32c-build.h"

ogs_sbi_request_t *sepp_n32c_handshake_build_exchange_capability(
        sepp_node_t *node)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    OpenAPI_sec_negotiate_req_data_t SecNegotiateReqData;
    OpenAPI_list_t *SupportedSecCapabilityList = NULL;

    ogs_assert(node);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_N32C_HANDSHAKE;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_EXCHANGE_CAPABILITY;

    memset(&SecNegotiateReqData, 0, sizeof(SecNegotiateReqData));
    SecNegotiateReqData.sender = sepp_self()->fqdn;

    SupportedSecCapabilityList = OpenAPI_list_create();
    if (!SupportedSecCapabilityList) {
        ogs_error("No SupportedSecCapabilityList");
        return NULL;
    }

    if (sepp_self()->security_capability.tls == true)
        OpenAPI_list_add(SupportedSecCapabilityList,
                (void *)OpenAPI_security_capability_TLS);
    if (sepp_self()->security_capability.prins == true)
        OpenAPI_list_add(SupportedSecCapabilityList,
                (void *)OpenAPI_security_capability_PRINS);

    if (SupportedSecCapabilityList->count)
        SecNegotiateReqData.supported_sec_capability_list =
            SupportedSecCapabilityList;
    else {
        ogs_error("No Security Capability [TLS:%d,PRINS:%d]",
                sepp_self()->security_capability.tls,
                sepp_self()->security_capability.prins);
        OpenAPI_list_free(SupportedSecCapabilityList);
        ogs_assert_if_reached();
    }

    message.SecNegotiateReqData = &SecNegotiateReqData;

    request = ogs_sbi_build_request(&message);
    ogs_expect(request);

    OpenAPI_list_free(
        SecNegotiateReqData.supported_sec_capability_list);

    return request;
}

ogs_sbi_request_t *sepp_n32c_handshake_build_termination(
        sepp_node_t *node)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    OpenAPI_sec_negotiate_req_data_t SecNegotiateReqData;
    OpenAPI_list_t *SupportedSecCapabilityList = NULL;

    ogs_assert(node);

    if (sepp_self()->negotiated_security_scheme !=
            OpenAPI_security_capability_TLS) {
        ogs_fatal("Not implemented [%s:%d]",
                OpenAPI_security_capability_ToString(
                    sepp_self()->negotiated_security_scheme),
                sepp_self()->negotiated_security_scheme);
        ogs_assert_if_reached();
    }

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_N32C_HANDSHAKE;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_EXCHANGE_CAPABILITY;

    memset(&SecNegotiateReqData, 0, sizeof(SecNegotiateReqData));
    SecNegotiateReqData.sender = sepp_self()->fqdn;

    SupportedSecCapabilityList = OpenAPI_list_create();
    if (!SupportedSecCapabilityList) {
        ogs_error("No SupportedSecCapabilityList");
        return NULL;
    }

    OpenAPI_list_add(SupportedSecCapabilityList,
            (void *)OpenAPI_security_capability_NONE);

    if (SupportedSecCapabilityList->count)
        SecNegotiateReqData.supported_sec_capability_list =
            SupportedSecCapabilityList;
    else {
        ogs_error("No Security Capability [TLS:%d,PRINS:%d]",
                sepp_self()->security_capability.tls,
                sepp_self()->security_capability.prins);
        OpenAPI_list_free(SupportedSecCapabilityList);
        ogs_assert_if_reached();
    }

    message.SecNegotiateReqData = &SecNegotiateReqData;

    request = ogs_sbi_build_request(&message);
    ogs_expect(request);

    OpenAPI_list_free(
        SecNegotiateReqData.supported_sec_capability_list);

    return request;
}
