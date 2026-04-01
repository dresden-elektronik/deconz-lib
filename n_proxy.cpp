#include "deconz/aps_controller.h"
#include "deconz/n_proxy.h"
#include "deconz/u_memory.h"

int N_GetProxy(N_Proxy *proxy)
{
    proxy->type = N_ProxyTypeNone;

    auto *apsCtrl = deCONZ::ApsController::instance();
    if (apsCtrl)
    {
        proxy->port = apsCtrl->getParameter(deCONZ::ParamHttpProxyPort);
        const QString host = apsCtrl->getParameter(deCONZ::ParamHttpProxy);
        if (host.size() < (int)sizeof(proxy->host))
        {
            U_memcpy(proxy->host, qPrintable(host), host.size());
            proxy->host[host.size()] = '\0';
            if (proxy->port != 0)
            {
                proxy->type = N_ProxyTypeHttp;
                return 1;
            }
        }
    }
    proxy->port = 0;
    proxy->host[0] = '\0';
    return 0;
}
