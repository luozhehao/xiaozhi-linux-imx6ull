



#include "mcp_server.h"
#include "websocket_client.h"
#include "board.h"
#include "volume_alsa.h"

 McpServer::McpServer() {
 }
 
 McpServer::~McpServer() {
     for (auto tool : tools_) {
         delete tool;
     }
     tools_.clear();
 }

void McpServer::AddCommonTools() {
    // *Important* To speed up the response time, we add the common tools to the beginning of
    // the tools list to utilize the prompt cache.
    // **重要** 为了提升响应速度，我们把常用的工具放在前面，利用 prompt cache 的特性。

    // Backup the original tools list and restore it after adding the common tools.
    auto original_tools = std::move(tools_);
    auto& board = Board::GetInstance();

    std::string j_str = board.GetDeviceStatusJson();
    // // 序列化
    if(!j_str.empty()){
        // std::string json_str = j.dump();
        std::cout << "### AddCommonTools, j_str: "<< j_str << std::endl;
    }else{
        std::cout << "### AddCommonTools, j_str: null" << std::endl;
    }


    // Do not add custom tools here.
    // Custom tools must be added in the board's InitializeTools function.

    AddTool("self.get_device_status",
        "Provides the real-time information of the device, including the current status of the audio speaker, screen, battery, network, etc.\n"
        "Use this tool for: \n"
        "1. Answering questions about current condition (e.g. what is the current volume of the audio speaker?)\n"
        "2. As the first step to control the device (e.g. turn up / down the volume of the audio speaker, etc.)",
        PropertyList(),
        [&board](const PropertyList& properties) -> ReturnValue {
            return board.GetDeviceStatusJson();
        });

    AddTool("self.audio_speaker.set_volume", 
        "Set the volume of the audio speaker. If the current volume is unknown, you must call `self.get_device_status` tool first and then call this tool.",
        PropertyList({
            Property("volume", kPropertyTypeInteger, 0, 100)
        }), 
        [&board](const PropertyList& properties) -> ReturnValue {
            // auto codec = board.GetAudioCodec();
            // codec->SetOutputVolume(properties["volume"].value<int>());
            auto amx = board.GetAudioAmx();
            // VolumeController volume;
            amx->set_volume(properties["volume"].value<int>());
            return true;
        });
    

    // Restore the original tools list to the end of the tools list
    tools_.insert(tools_.end(), original_tools.begin(), original_tools.end());
}


void McpServer::AddTool(McpTool* tool) {
    // Prevent adding duplicate tools
    if (std::find_if(tools_.begin(), tools_.end(), [tool](const McpTool* t) { return t->name() == tool->name(); }) != tools_.end()) {
        // ESP_LOGW(TAG, "Tool %s already added", tool->name().c_str());
        return;
    }
    // ESP_LOGI(TAG, "Add tool: %s", tool->name().c_str());
    tools_.push_back(tool);
}

void McpServer::AddTool(const std::string& name, const std::string& description, const PropertyList& properties, std::function<ReturnValue(const PropertyList&)> callback) {
    AddTool(new McpTool(name, description, properties, callback));
}

//2025.11.02 03:18 
// params.is_object()
void McpServer::ParseCapabilities(json& capabilities) {
    auto vision = capabilities["vision"];          //cJSON_GetObjectItem(capabilities, "vision");
    if (vision.is_object()) {
        auto url = vision["url"];
        //cJSON_GetObjectItem(vision, "url");
        auto token = vision["token"];          //cJSON_GetObjectItem(vision, "token");
        if (url.is_string()) {
            std::cout << "### url is string" << std::endl;
            //  auto camera = Board::GetInstance().GetCamera();
            //  if (camera) {
            //      std::string url_str = std::string(url->valuestring);
            //      std::string token_str;
            //      if (cJSON_IsString(token)) {
            //          token_str = std::string(token->valuestring);
            //      }
            //      camera->SetExplainUrl(url_str, token_str);
            //  }
        }
    }
}

void McpServer::ParseCapabilities(const cJSON* capabilities) {
    auto vision = cJSON_GetObjectItem(capabilities, "vision");
    if (cJSON_IsObject(vision)) {
        auto url = cJSON_GetObjectItem(vision, "url");
        auto token = cJSON_GetObjectItem(vision, "token");
        if (cJSON_IsString(url)) {
        //  auto camera = Board::GetInstance().GetCamera();
        //  if (camera) {
        //      std::string url_str = std::string(url->valuestring);
        //      std::string token_str;
        //      if (cJSON_IsString(token)) {
        //          token_str = std::string(token->valuestring);
        //      }
        //      camera->SetExplainUrl(url_str, token_str);
        //  }
        }
    }
}


void McpServer::ReplyResult(int id, const std::string& result){
    printf("### ReplyResult \n");
    std::string payload = "{\"jsonrpc\":\"2.0\",\"id\":";
    payload += std::to_string(id) + ",\"result\":";
    payload += result;
    payload += "}";
    std::cout << "### payload: "<< payload << std::endl;
    // // 创建 JSON（更直观）
    // json j;
    // j["type"] = "mcp";
    // j["payload"] = payload;
    // j["session_id"] = mcp_session_id;
    // // 序列化
    // std::string json_str = j.dump();
    // std::cout << "### json_str: "<< json_str << std::endl;

    // std::string message = "{\"session_id\":\"" + mcp_session_id + "\",\"type\":\"mcp\",\"payload\":" + payload + "}";
    // // SendText(message);
    // websocket_send_text(message.data(), message.size());
    // Application::GetInstance().SendMcpMessage(payload);
    SendMcpMessage(payload);
}

void McpServer::SendMcpMessage(const std::string& payload){

    std::string message = "{\"session_id\":\"" + mcp_session_id + "\",\"type\":\"mcp\",\"payload\":" + payload + "}";
    // SendText(message);
    websocket_send_text(message.data(), message.size());
}

void McpServer::ReplyError(int id, const std::string& message){
    printf("### ReplyError ...  \n");
    std::string payload = "{\"jsonrpc\":\"2.0\",\"id\":";
    payload += std::to_string(id);
    payload += ",\"error\":{\"message\":\"";
    payload += message;
    payload += "\"}}";
    // Application::GetInstance().SendMcpMessage(payload);
    SendMcpMessage(payload);
}

void McpServer::GetToolsList(int id, const std::string& cursor){
    printf("### GetToolsList ... 001 \n");
    const int max_payload_size = 8000;
    std::string json = "{\"tools\":[";

    bool found_cursor = cursor.empty();
    auto it = tools_.begin();
    std::string next_cursor = "";

    while (it != tools_.end()) {
        // 如果我们还没有找到起始位置，继续搜索
        if (!found_cursor) {
            if ((*it)->name() == cursor) {
                found_cursor = true;
            } else {
                ++it;
                continue;
            }
        }
        
        // 添加tool前检查大小
        std::string tool_json = (*it)->to_json() + ",";
        if (json.length() + tool_json.length() + 30 > max_payload_size) {
            // 如果添加这个tool会超出大小限制，设置next_cursor并退出循环
            next_cursor = (*it)->name();
            break;
        }
        
        json += tool_json;
        ++it;
    }

    if (json.back() == ',') {
        json.pop_back();
    }

    if (json.back() == '[' && !tools_.empty()) {
        // 如果没有添加任何tool，返回错误
        // ESP_LOGE(TAG, "tools/list: Failed to add tool %s because of payload size limit", next_cursor.c_str());
        ReplyError(id, "Failed to add tool " + next_cursor + " because of payload size limit");
        return;
    }

    if (next_cursor.empty()) {
        json += "]}";
    } else {
        json += "],\"nextCursor\":\"" + next_cursor + "\"}";
    }

    ReplyResult(id, json);
}



void McpServer::DoToolCall(int id, const std::string& tool_name, const cJSON* tool_arguments, int stack_size){
    auto tool_iter = std::find_if(tools_.begin(), tools_.end(), 
                                [&tool_name](const McpTool* tool) { 
                                    return tool->name() == tool_name; 
                                });
    
    if (tool_iter == tools_.end()) {
        // ESP_LOGE(TAG, "tools/call: Unknown tool: %s", tool_name.c_str());
        ReplyError(id, "Unknown tool: " + tool_name);
        return;
    }

    PropertyList arguments = (*tool_iter)->properties();
    try {
        for (auto& argument : arguments) {
            bool found = false;
            if (cJSON_IsObject(tool_arguments)) {
                auto value = cJSON_GetObjectItem(tool_arguments, argument.name().c_str());
                if (argument.type() == kPropertyTypeBoolean && cJSON_IsBool(value)) {
                    argument.set_value<bool>(value->valueint == 1);
                    found = true;
                } else if (argument.type() == kPropertyTypeInteger && cJSON_IsNumber(value)) {
                    argument.set_value<int>(value->valueint);
                    found = true;
                } else if (argument.type() == kPropertyTypeString && cJSON_IsString(value)) {
                    argument.set_value<std::string>(value->valuestring);
                    found = true;
                }
            }

            if (!argument.has_default_value() && !found) {
                // ESP_LOGE(TAG, "tools/call: Missing valid argument: %s", argument.name().c_str());
                ReplyError(id, "Missing valid argument: " + argument.name());
                return;
            }
        }
    } catch (const std::exception& e) {
        // ESP_LOGE(TAG, "tools/call: %s", e.what());
        ReplyError(id, e.what());
        return;
    }

    // Start a task to receive data with stack size
    // esp_pthread_cfg_t cfg = esp_pthread_get_default_config();
    // cfg.thread_name = "tool_call";
    // cfg.stack_size = stack_size;
    // cfg.prio = 1;
    // esp_pthread_set_cfg(&cfg);

    // Use a thread to call the tool to avoid blocking the main thread
    tool_call_thread_ = std::thread([this, id, tool_iter, arguments = std::move(arguments)]() {
        try {
            ReplyResult(id, (*tool_iter)->Call(arguments));
        } catch (const std::exception& e) {
            // ESP_LOGE(TAG, "tools/call: %s", e.what());
            ReplyError(id, e.what());
        }
    });
    tool_call_thread_.detach();
}

void McpServer::DoToolCall(int id, const std::string& tool_name, json& tool_arguments, int stack_size){

    std::cout << "DoToolCall ... 001 " << std::endl;
    auto tool_iter = std::find_if(tools_.begin(), tools_.end(), 
                                [&tool_name](const McpTool* tool) { 
                                    return tool->name() == tool_name; 
                                });
    
    if (tool_iter == tools_.end()) {
        // ESP_LOGE(TAG, "tools/call: Unknown tool: %s", tool_name.c_str());
        ReplyError(id, "Unknown tool: " + tool_name);
        return;
    }
    std::cout << "DoToolCall ... 002 " << std::endl;

    PropertyList arguments = (*tool_iter)->properties();
    try {
        for (auto& argument : arguments) {
            bool found = false;
            // if (cJSON_IsObject(tool_arguments)) {
            if (tool_arguments.is_object()) {
                auto value = tool_arguments[argument.name().c_str()];          // cJSON_GetObjectItem(tool_arguments, argument.name().c_str());
                if (argument.type() == kPropertyTypeBoolean && value.is_boolean()) {          //cJSON_IsBool(value)) {
                    argument.set_value<bool> (value.get<bool>() == 1);                                  //(value->valueint == 1);
                    found = true;
                } else if (argument.type() == kPropertyTypeInteger && value.is_number() ) {    //cJSON_IsNumber(value)) {
                    argument.set_value<int>(value.get<int>());  // (value->valueint);
                    found = true;
                } else if (argument.type() == kPropertyTypeString && value.is_string() ) {   //cJSON_IsString(value)) {
                    argument.set_value<std::string>(value.get<std::string>());
                    found = true;
                }
            }

            if (!argument.has_default_value() && !found) {
                // ESP_LOGE(TAG, "tools/call: Missing valid argument: %s", argument.name().c_str());
                ReplyError(id, "Missing valid argument: " + argument.name());
                return;
            }
        }
    } catch (const std::exception& e) {
        // ESP_LOGE(TAG, "tools/call: %s", e.what());
        ReplyError(id, e.what());
        return;
    }
    std::cout << "DoToolCall ... 005 " << std::endl;

    // Start a task to receive data with stack size
    // esp_pthread_cfg_t cfg = esp_pthread_get_default_config();
    // cfg.thread_name = "tool_call";
    // cfg.stack_size = stack_size;
    // cfg.prio = 1;
    // esp_pthread_set_cfg(&cfg);

    // Use a thread to call the tool to avoid blocking the main thread
    tool_call_thread_ = std::thread([this, id, tool_iter, arguments = std::move(arguments)]() {
        try {
            std::cout << "tool_call_thread_ ... 001 " << std::endl;
            ReplyResult(id, (*tool_iter)->Call(arguments));
            std::cout << "tool_call_thread_ ... 009 " << std::endl;
        } catch (const std::exception& e) {
            // ESP_LOGE(TAG, "tools/call: %s", e.what());
            ReplyError(id, e.what());
        }
    });
    tool_call_thread_.detach();
    std::cout << "DoToolCall ... 009 " << std::endl;
}


void McpServer::ParseMessage(json& json) {
    std::cout << "ParseMessage ... 001 " << std::endl;
    // Check JSONRPC version
    auto version = json["jsonrpc"];    //cJSON_GetObjectItem(json, "jsonrpc");
    std::string version_str = version.get<std::string>();
    if (version == nullptr || !version.is_string() || version_str != "2.0") {
        // ESP_LOGE(TAG, "Invalid JSONRPC version: %s", version ? version->valuestring : "null");
        return;
    }
    std::cout << "ParseMessage ... 002 " << std::endl;
    
    // Check method
    auto method = json["method"];          //cJSON_GetObjectItem(json, "method");
    if (method == nullptr || !method.is_string()) {
        // ESP_LOGE(TAG, "Missing method");
        return;
    }
    
    auto method_str =  method.get<std::string>();   //std::string(method->valuestring);
    if (method_str.find("notifications") == 0) {
        return;
    }
    
    // Check params
    auto params = json["params"];    // cJSON_GetObjectItem(json, "params");
    if (params != nullptr && !params.is_object()) {
        // ESP_LOGE(TAG, "Invalid params for method: %s", method_str.c_str());
        return;
    }

    auto id =  json["id"];     // cJSON_GetObjectItem(json, "id");
    if (id == nullptr || !id.is_number()) {
        // ESP_LOGE(TAG, "Invalid id for method: %s", method_str.c_str());
        return;
    }
    int id_int = json["id"].get<int>();        //id->valueint;
    std::cout << "ParseMessage ... 005 " << std::endl;
    if (method_str == "initialize") {
        printf("### ParseMeg, method_str == initialize\n");
        if (params.is_object()) {
            auto capabilities = params["capabilities"];  // cJSON_GetObjectItem(params, "capabilities");
            if (capabilities.is_object()) {
                ParseCapabilities(capabilities);
            }
        }
        // auto app_desc = esp_app_get_description();
        std::string message = "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{}},\"serverInfo\":{\"name\":\"" BOARD_NAME "\",\"version\":\"";
        // message += app_desc->version;
        message += "1.0.0";
        message += "\"}}";
        ReplyResult(id_int, message);
    } else if (method_str == "tools/list") {
        std::cout << "### method_str == tools.list ...001 " << std::endl;
        std::string cursor_str = "";
        if (params != nullptr) {
            auto cursor = params[ "cursor"];
            // std::cout << "### params, cursor = " << cursor.get<std::string>() << std::endl;
            //cJSON_GetObjectItem(params, "cursor");
            if (cursor.is_string()) {
                cursor_str = cursor.get<std::string>();  //std::string(cursor->valuestring);
            }
        }
        GetToolsList(id_int, cursor_str);
        std::cout << "### method_str == tools.list ...009 " << std::endl;
    } else if (method_str == "tools/call") {
        std::cout << "### method_str == tools.call ...001 " << std::endl;
        if (!params.is_object() ) {
            // ESP_LOGE(TAG, "tools/call: Missing params");
            ReplyError(id_int, "Missing params");
            return;
        }
        std::cout << "### method_str == tools.call ...002 " << std::endl;
        auto tool_name = params["name"];         //cJSON_GetObjectItem(params, "name");
        if (!tool_name.is_string()) {
            // ESP_LOGE(TAG, "tools/call: Missing name");
            ReplyError(id_int, "Missing name");
            return;
        }
        std::cout << "### method_str == tools.call ...003 " << std::endl;
        auto tool_arguments = params["arguments"]; // cJSON_GetObjectItem(params, "arguments");
        if (tool_arguments != nullptr && !tool_arguments.is_object()) {
            // ESP_LOGE(TAG, "tools/call: Invalid arguments");
            ReplyError(id_int, "Invalid arguments");
            return;
        }
        std::cout << "### method_str == tools.call ...004, tool_name: "<<  tool_name.get<std::string>() << std::endl;
        // auto stack_size = params["stackSize"];   // cJSON_GetObjectItem(params, "stackSize");
        // if (stack_size != nullptr && !stack_size.is_number()) {
        //     // ESP_LOGE(TAG, "tools/call: Invalid stackSize");
        //     ReplyError(id_int, "Invalid stackSize");
        //     return;
        // }
        // std::cout << "### method_str == tools.call ...005 " << std::endl;

        // int stack_size_int = params["stackSize"].get<int>();
        // std::cout << "### stack_size_int =  " << stack_size_int << std::endl;

        DoToolCall(id_int, tool_name.get<std::string>(), tool_arguments, DEFAULT_TOOLCALL_STACK_SIZE);
        // stack_size_int ? stack_size_int : DEFAULT_TOOLCALL_STACK_SIZE
        std::cout << "### method_str == tools.call ...009 " << std::endl;
    } else {
        // ESP_LOGE(TAG, "Method not implemented: %s", method_str.c_str());
        ReplyError(id_int, "Method not implemented: " + method_str);
    }
    std::cout << "ParseMessage ... 009 " << std::endl;
}


void McpServer::ParseMessage(const cJSON* json) {
    std::cout << "ParseMessage ... 001 " << std::endl;
    // Check JSONRPC version
    auto version = cJSON_GetObjectItem(json, "jsonrpc");
    if (version == nullptr || !cJSON_IsString(version) || strcmp(version->valuestring, "2.0") != 0) {
        // ESP_LOGE(TAG, "Invalid JSONRPC version: %s", version ? version->valuestring : "null");
        return;
    }
    std::cout << "ParseMessage ... 002 " << std::endl;
    
    // Check method
    auto method = cJSON_GetObjectItem(json, "method");
    if (method == nullptr || !cJSON_IsString(method)) {
        // ESP_LOGE(TAG, "Missing method");
        return;
    }
    
    auto method_str = std::string(method->valuestring);
    if (method_str.find("notifications") == 0) {
        return;
    }
    
    // Check params
    auto params = cJSON_GetObjectItem(json, "params");
    if (params != nullptr && !cJSON_IsObject(params)) {
        // ESP_LOGE(TAG, "Invalid params for method: %s", method_str.c_str());
        return;
    }

    auto id = cJSON_GetObjectItem(json, "id");
    if (id == nullptr || !cJSON_IsNumber(id)) {
        // ESP_LOGE(TAG, "Invalid id for method: %s", method_str.c_str());
        return;
    }
    auto id_int = id->valueint;
    std::cout << "ParseMessage ... 005 " << std::endl;
    if (method_str == "initialize") {
        printf("### ParseMeg, method_str == initialize\n");
        if (cJSON_IsObject(params)) {
            auto capabilities = cJSON_GetObjectItem(params, "capabilities");
            if (cJSON_IsObject(capabilities)) {
                ParseCapabilities(capabilities);
            }
        }
        // auto app_desc = esp_app_get_description();
        std::string message = "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{}},\"serverInfo\":{\"name\":\"" BOARD_NAME "\",\"version\":\"";
        // message += app_desc->version;
        message += "1.0.0";
        message += "\"}}";
        ReplyResult(id_int, message);
    } else if (method_str == "tools/list") {
        std::string cursor_str = "";
        if (params != nullptr) {
            auto cursor = cJSON_GetObjectItem(params, "cursor");
            if (cJSON_IsString(cursor)) {
                cursor_str = std::string(cursor->valuestring);
            }
        }
        GetToolsList(id_int, cursor_str);
    } else if (method_str == "tools/call") {
        if (!cJSON_IsObject(params)) {
            // ESP_LOGE(TAG, "tools/call: Missing params");
            ReplyError(id_int, "Missing params");
            return;
        }
        auto tool_name = cJSON_GetObjectItem(params, "name");
        if (!cJSON_IsString(tool_name)) {
            // ESP_LOGE(TAG, "tools/call: Missing name");
            ReplyError(id_int, "Missing name");
            return;
        }
        auto tool_arguments = cJSON_GetObjectItem(params, "arguments");
        if (tool_arguments != nullptr && !cJSON_IsObject(tool_arguments)) {
            // ESP_LOGE(TAG, "tools/call: Invalid arguments");
            ReplyError(id_int, "Invalid arguments");
            return;
        }
        auto stack_size = cJSON_GetObjectItem(params, "stackSize");
        if (stack_size != nullptr && !cJSON_IsNumber(stack_size)) {
            // ESP_LOGE(TAG, "tools/call: Invalid stackSize");
            ReplyError(id_int, "Invalid stackSize");
            return;
        }
        DoToolCall(id_int, std::string(tool_name->valuestring), tool_arguments, stack_size ? stack_size->valueint : DEFAULT_TOOLCALL_STACK_SIZE);
    } else {
        // ESP_LOGE(TAG, "Method not implemented: %s", method_str.c_str());
        ReplyError(id_int, "Method not implemented: " + method_str);
    }
    std::cout << "ParseMessage ... 009 " << std::endl;
}