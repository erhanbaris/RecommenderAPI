#pragma once

#include <core/server/action/BaseAction.h>

namespace core {
    namespace server {
        namespace action {
            class SearchAction: public BaseAction {
            public:
                STR_TYPE Url() override;
                ResponseInfo Execute(RequestInfo *info) override;
                short MethodInfo() override;
                BaseAction* CreateObject() override;
            };
        }
    }
}
