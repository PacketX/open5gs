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

#include "context.h"

static sepp_context_t self;

int __sepp_log_domain;

static OGS_POOL(sepp_assoc_pool, sepp_assoc_t);

static int context_initialized = 0;

static int max_num_of_sepp_assoc = 0;

void sepp_context_init(void)
{
    ogs_assert(context_initialized == 0);

    /* Initialize SEPP context */
    memset(&self, 0, sizeof(sepp_context_t));

    ogs_log_install_domain(&__sepp_log_domain, "sepp", ogs_core()->log.level);

    /* Peer SEPP List */
    ogs_list_init(&self.peer_list);

#define MAX_NUM_OF_SEPP_ASSOC 8
    max_num_of_sepp_assoc = ogs_app()->max.ue * MAX_NUM_OF_SEPP_ASSOC;

    ogs_pool_init(&sepp_assoc_pool, max_num_of_sepp_assoc);

    context_initialized = 1;
}

void sepp_context_final(void)
{
    ogs_assert(context_initialized == 1);

    ogs_sbi_nf_instance_remove_all(&sepp_self()->peer_list);

    sepp_assoc_remove_all();

    ogs_pool_final(&sepp_assoc_pool);

    context_initialized = 0;
}

sepp_context_t *sepp_self(void)
{
    return &self;
}

static int sepp_context_prepare(void)
{
    return OGS_OK;
}

static int sepp_context_validation(void)
{
    return OGS_OK;
}

int sepp_context_parse_config(void)
{
    int rv;
    yaml_document_t *document = NULL;
    ogs_yaml_iter_t root_iter;

    document = ogs_app()->document;
    ogs_assert(document);

    rv = sepp_context_prepare();
    if (rv != OGS_OK) return rv;

    ogs_yaml_iter_init(&root_iter, document);
    while (ogs_yaml_iter_next(&root_iter)) {
        const char *root_key = ogs_yaml_iter_key(&root_iter);
        ogs_assert(root_key);
        if (!strcmp(root_key, "sepp")) {
            ogs_yaml_iter_t sepp_iter;
            ogs_yaml_iter_recurse(&root_iter, &sepp_iter);
            while (ogs_yaml_iter_next(&sepp_iter)) {
                const char *sepp_key = ogs_yaml_iter_key(&sepp_iter);
                ogs_assert(sepp_key);
                if (!strcmp(sepp_key, "sbi")) {
                    /* handle config in sbi library */
                } else if (!strcmp(sepp_key, "service_name")) {
                    /* handle config in sbi library */
                } else if (!strcmp(sepp_key, "discovery")) {
                    /* handle config in sbi library */
                } else if (!strcmp(sepp_key, "peer")) {
                    ogs_yaml_iter_t peer_array, peer_iter;
                    ogs_yaml_iter_recurse(&sepp_iter, &peer_array);
                    do {
                        ogs_sbi_nf_instance_t *nf_instance = NULL;
                        ogs_sbi_client_t *client = NULL;
                        ogs_sockaddr_t *addr = NULL;
                        int family = AF_UNSPEC;
                        int i, num = 0;
                        const char *hostname[OGS_MAX_NUM_OF_HOSTNAME];
                        uint16_t port = ogs_sbi_client_default_port();

                        if (ogs_yaml_iter_type(&peer_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&peer_iter, &peer_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&peer_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&peer_array))
                                break;
                            ogs_yaml_iter_recurse(&peer_array, &peer_iter);
                        } else if (ogs_yaml_iter_type(&peer_array) ==
                                YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&peer_iter)) {
                            const char *peer_key =
                                ogs_yaml_iter_key(&peer_iter);
                            ogs_assert(peer_key);
                            if (!strcmp(peer_key, "family")) {
                                const char *v = ogs_yaml_iter_value(&peer_iter);
                                if (v) family = atoi(v);
                                if (family != AF_UNSPEC &&
                                    family != AF_INET && family != AF_INET6) {
                                    ogs_warn("Ignore family(%d) : "
                                        "AF_UNSPEC(%d), "
                                        "AF_INET(%d), AF_INET6(%d) ",
                                        family, AF_UNSPEC, AF_INET, AF_INET6);
                                    family = AF_UNSPEC;
                                }
                            } else if (!strcmp(peer_key, "addr") ||
                                    !strcmp(peer_key, "name")) {
                                ogs_yaml_iter_t hostname_iter;
                                ogs_yaml_iter_recurse(&peer_iter,
                                        &hostname_iter);
                                ogs_assert(ogs_yaml_iter_type(&hostname_iter) !=
                                    YAML_MAPPING_NODE);

                                do {
                                    if (ogs_yaml_iter_type(&hostname_iter) ==
                                            YAML_SEQUENCE_NODE) {
                                        if (!ogs_yaml_iter_next(&hostname_iter))
                                            break;
                                    }

                                    ogs_assert(num < OGS_MAX_NUM_OF_HOSTNAME);
                                    hostname[num++] =
                                        ogs_yaml_iter_value(&hostname_iter);
                                } while (
                                    ogs_yaml_iter_type(&hostname_iter) ==
                                        YAML_SEQUENCE_NODE);
                            } else if (!strcmp(peer_key, "port")) {
                                const char *v = ogs_yaml_iter_value(&peer_iter);
                                if (v) port = atoi(v);
                            } else if (!strcmp(peer_key, "advertise")) {
                                /* Nothing in client */
                            } else
                                ogs_warn("unknown key `%s`", peer_key);
                        }

                        addr = NULL;
                        for (i = 0; i < num; i++) {
                            rv = ogs_addaddrinfo(&addr,
                                    family, hostname[i], port, 0);
                            ogs_assert(rv == OGS_OK);
                        }

                        ogs_filter_ip_version(&addr,
                                ogs_app()->parameter.no_ipv4,
                                ogs_app()->parameter.no_ipv6,
                                ogs_app()->parameter.prefer_ipv4);

                        if (addr == NULL) continue;

                        nf_instance = ogs_sbi_nf_instance_add(
                                &sepp_self()->peer_list);
                        ogs_assert(nf_instance);
                        ogs_sbi_nf_instance_set_type(
                                nf_instance, OpenAPI_nf_type_SEPP);

                        client = ogs_sbi_client_add(
                                    ogs_sbi_client_default_scheme(), addr);
                        ogs_assert(client);
                        OGS_SBI_SETUP_CLIENT(nf_instance, client);

                        ogs_freeaddrinfo(addr);

                    } while (ogs_yaml_iter_type(&peer_array) ==
                            YAML_SEQUENCE_NODE);
                } else if (!strcmp(sepp_key, "info")) {
                    ogs_sbi_nf_instance_t *nf_instance = NULL;
                    ogs_sbi_nf_info_t *nf_info = NULL;
                    ogs_sbi_sepp_info_t *sepp_info = NULL;

                    ogs_yaml_iter_t info_iter;
                    ogs_yaml_iter_recurse(&sepp_iter, &info_iter);

                    nf_instance = ogs_sbi_self()->nf_instance;
                    ogs_assert(nf_instance);

                    nf_info = ogs_sbi_nf_info_add(
                                &nf_instance->nf_info_list,
                                    OpenAPI_nf_type_SEPP);
                    ogs_assert(nf_info);

                    sepp_info = &nf_info->sepp;

                    while (ogs_yaml_iter_next(&info_iter)) {
                        const char *info_key =
                            ogs_yaml_iter_key(&info_iter);
                        ogs_assert(info_key);
                        if (!strcmp(info_key, "port")) {
                            ogs_yaml_iter_t port_iter;
                            ogs_yaml_iter_recurse(&info_iter, &port_iter);
                            while (ogs_yaml_iter_next(&port_iter)) {
                                const char *port_key =
                                    ogs_yaml_iter_key(&port_iter);
                                ogs_assert(port_key);
                                if (!strcmp(port_key, "http")) {
                                    const char *v =
                                        ogs_yaml_iter_value(&port_iter);
                                    if (v) {
                                        sepp_info->http.presence = true;
                                        sepp_info->http.port = atoi(v);
                                    }
                                } else if (!strcmp(port_key, "https")) {
                                    const char *v =
                                        ogs_yaml_iter_value(&port_iter);
                                    if (v) {
                                        sepp_info->https.presence = true;
                                        sepp_info->https.port = atoi(v);
                                    }
                                } else
                                    ogs_warn("unknown key `%s`", port_key);
                            }
                        } else
                            ogs_warn("unknown key `%s`", info_key);
                    }
                } else
                    ogs_warn("unknown key `%s`", sepp_key);
            }
        }
    }

    rv = sepp_context_validation();
    if (rv != OGS_OK) return rv;

    return OGS_OK;
}

sepp_assoc_t *sepp_assoc_add(ogs_sbi_stream_t *stream)
{
    sepp_assoc_t *assoc = NULL;

    ogs_assert(stream);

    ogs_pool_alloc(&sepp_assoc_pool, &assoc);
    if (!assoc) {
        ogs_error("Maximum number of association[%d] reached",
                    max_num_of_sepp_assoc);
        return NULL;
    }
    memset(assoc, 0, sizeof *assoc);

    assoc->stream = stream;

    ogs_list_add(&self.assoc_list, assoc);

    return assoc;
}

void sepp_assoc_remove(sepp_assoc_t *assoc)
{
    ogs_assert(assoc);

    ogs_list_remove(&self.assoc_list, assoc);

    if (assoc->client)
        ogs_sbi_client_remove(assoc->client);
    if (assoc->nrf_client)
        ogs_sbi_client_remove(assoc->nrf_client);

    ogs_pool_free(&sepp_assoc_pool, assoc);
}

void sepp_assoc_remove_all(void)
{
    sepp_assoc_t *assoc = NULL, *next_assoc = NULL;

    ogs_list_for_each_safe(&self.assoc_list, next_assoc, assoc)
        sepp_assoc_remove(assoc);
}

sepp_assoc_t *sepp_assoc_find(uint32_t index)
{
    return ogs_pool_find(&sepp_assoc_pool, index);
}
