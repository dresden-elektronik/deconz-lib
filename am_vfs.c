#include "deconz/u_assert.h"
#include "deconz/am_vfs.h"


am_string AM_UrlElementAt(am_url_parse *up, unsigned idx)
{
    am_string result;
    result.size = 0;
    result.data = 0;

    if (up->element_count <= idx)
        return result;

    unsigned i;
    unsigned pos = 0;

    for (i = 0; i < idx; i++)
    {
        pos += up->elements[i] + 1; // +1 for '/'
    }

    result.data = &up->url.data[pos];
    result.size = up->elements[i];

    return result;
}

int AM_ParseUrl(am_url_parse *up)
{
    up->element_count = 0;
    up->elements[0] = 0;

    for (unsigned i = 0; i < up->url.size; i++)
    {
        if (up->url.data[i] == '/')
        {
            U_ASSERT(AM_MAX_URL_ELEMENTS >= up->element_count + 1);
            if (AM_MAX_URL_ELEMENTS < up->element_count + 1)
                return 0; // TODO too many elements

            up->element_count++;
            up->elements[up->element_count] = 0;
        }
        else
        {
            up->elements[up->element_count] += 1;
        }
    }

    if (up->url.size)
        up->element_count++;

    return 1;
}

int AM_ParseListDirectoryRequest(struct am_api_functions *am, struct am_message *msg, am_ls_dir_req *req)
{
    req->tag = am->msg_get_u16(msg);
    req->url_parse.url = am->msg_get_string(msg);
    req->req_index = am->msg_get_u32(msg);
    req->max_count = am->msg_get_u32(msg);

    if (msg->status == AM_MSG_STATUS_OK)
        if (AM_ParseUrl(&req->url_parse))
            return msg->status;

    return AM_MSG_STATUS_ERROR;
}

int AM_ParseReadEntryRequest(struct am_api_functions *am, struct am_message *msg, am_read_entry_req *req)
{
    req->tag = am->msg_get_u16(msg);
    req->url_parse.url = am->msg_get_string(msg);

    if (msg->status == AM_MSG_STATUS_OK)
        if (AM_ParseUrl(&req->url_parse))
            return msg->status;

    return AM_MSG_STATUS_ERROR;
}
