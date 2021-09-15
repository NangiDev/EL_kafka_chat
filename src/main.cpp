#include "global.h"
#include "my_kafka.h"
#include <vector>

using namespace std;

vector<cppkafka::TopicMetadata> roomList  = {};
vector<std::string> historyList = {};

static char buf[1024] = "";
static int item_current = 0;

void HistoryListBox(vector<string> *items) {
    if (ImGui::ListBoxHeader("##HistoryList", ImVec2(0, -1)))
    {
        ImGui::SetScrollY(0.0f);
        for (auto item : *items)
        {
            string text = item;
            if (text.rfind("YOU ->", 0) == 0)
                ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - text.length() * 7);
            ImGui::Text(text.c_str());
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::ListBoxFooter();
    }
}

void RoomListBox(vector<cppkafka::TopicMetadata> *items) {
    if (ImGui::ListBoxHeader("##RoomList", ImVec2(0, -1)))
    {
        for (auto item : *items)
        {
            string name = item.get_name();
            if (name.rfind(ROOM_PREFIX, 0) == 0)
                ImGui::Selectable(name.substr(ROOM_PREFIX.length(), name.length()).c_str());
        }
        ImGui::ListBoxFooter();
    }
}

void roomChild() {
    ImGui::BeginChild("##Rooms", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, ImGui::GetContentRegionAvail().y * 1.0f));
    ImGui::PushItemWidth(-1);
    RoomListBox(&roomList);
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void historyChild() {
    ImGui::BeginChild("##History", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.95f));
    ImGui::PushItemWidth(-1);
    HistoryListBox(&historyList);
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void messagingChild() {
    ImGui::BeginChild("##Messageing", ImVec2(0, ImGui::GetContentRegionAvail().y * 1.0f));
    ImGui::PushItemWidth(-1);
    if (ImGui::Button("Send")) {
        Publisher::publishMessage(buf);
        buf[0] = '\0';
    }
    ImGui::SameLine();
    if (ImGui::InputText("##MessageInput", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        ImGui::SetKeyboardFocusHere(0);
        Publisher::publishMessage(buf);
        buf[0] = '\0';
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void chatChild() {
    ImGui::BeginChild("##Chat", ImVec2(ImGui::GetContentRegionAvail().x * 1.0f, 0));
    historyChild();
    messagingChild();
    ImGui::EndChild();
}

int main(int, char**)
{
    setup();

    std::thread t1(Reciever::start, &roomList, &historyList);

    // Main loop
    while (!glfwWindowShouldClose(window) && !gQuit)
    {
        startAndPoll();

        {
            ImGui::Begin("##Main", NULL, getImguiFlags());
            ImGui::PushItemWidth(-1);
            roomChild();
            ImGui::SameLine();
            chatChild();
            ImGui::PopItemWidth();
            ImGui::End();
        }

        render();
    }

    Reciever::stop();

    t1.join();
    cleanup();

    return 0;
}