#pragma once

#include <optional>
#include <string>
#include <vector>

namespace storm
{
class QuestFileReader
{
public:
    struct UserData {
        /// @note Constructor is needed for emplace_back
        UserData(std::string const& id, std::string const& str) : id {id}, str {str} {}

        std::string id;
        std::string str;
    };

    struct QuestDescribe {
        struct TextDescribe {
            std::string id;
            std::string str;
        };

        std::string               questID;
        std::string               title;
        std::vector<TextDescribe> texts;
    };

public:
    QuestFileReader();
    ~QuestFileReader();

    void SetQuestTextFileName(std::string_view const& fileName);

    bool GetQuestTitle(std::string_view const& questId, std::string_view const& questUniqueID, std::string& buffer);

    std::string GetRecordText(std::string_view const& questID, std::string_view const& textID, std::string_view const& userData);

    static bool        AssembleStringToBuffer(std::string_view const& src, std::string& buffer, std::vector<UserData> const& userData);
    static std::string GetInsertStringByID(std::string_view const& id, std::vector<UserData> const& userData);
    static void        FillUserDataList(std::string_view const& strData, std::vector<UserData>& userData);

private:
    void ReadUserData(std::string_view const& questID, int32_t recordIndex);

    void AddQuestsFromBuffer(std::string_view const& srcBuffer);
    void AddQuestToList(std::string_view const& questID, std::string_view const& titleText);
    void AddTextToQuest(std::string_view const& questID, std::string_view const& textID, std::string_view const& questText);
    std::optional<size_t> FindQuestByID(std::string_view const& questID);
    std::optional<size_t> FindTextByID(QuestDescribe& nQuest, std::string_view const& textID);

private:
    std::vector<std::string> questFileNames_;

    std::vector<QuestDescribe> quests_;

    std::string           curQuestID_;
    std::vector<UserData> questData_;
};
}  // namespace storm
