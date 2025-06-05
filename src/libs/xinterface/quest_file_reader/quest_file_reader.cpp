#include "quest_file_reader.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>

#include <libs/core/core.h>
#include <libs/util/storm_assert.h>

namespace
{

enum class Token {
    Invalid,
    Quest,
    Text,

    Unknown,
};

std::map<std::string_view, Token> const kTokenMap = {{"#QUEST", Token::Quest}, {"#TEXT", Token::Text}};

/// @brief Read text until whitespace or newline
/// @param[in] str Text buffer to read from
/// @param[in,out] it An iterator pointing to place where reading needs to be started,
///                   will be set to the character right after the text that was read from str
/// @return Text that was read
/// @note If there is whitespaces in the beginning, they will be trimmed before reading
std::string_view ReadWord(std::string_view const& str, std::string_view::const_iterator& it)
{
    auto begin = std::find_if(it, str.cend(), [](char const ch) { return !isspace(static_cast<unsigned char>(ch)); });
    auto end   = std::find_if(begin, str.cend(), [](char const ch) { return isspace(static_cast<unsigned char>(ch)); });

    it = end;

    return std::string_view(begin, end);
}

/// @brief Read text until new token starts ('#' symbol)
/// @param[in] str Text buffer to read from
/// @param[in,out] it An iterator pointing to place where reading needs to be started,
///                   will be set to the '#' character right after the text that was read from str
/// @return Text that was read
/// @note If there is whitespaces in the beginning, they will be trimmed before reading. Trailing whitespaces will be
/// trimmed too.
std::string_view ReadText(std::string_view const& str, std::string_view::const_iterator& it)
{
    auto begin = std::find_if(it, str.cend(), [](char const ch) { return !isspace(static_cast<unsigned char>(ch)); });
    auto end   = std::find_if(begin, str.cend(), [](char const ch) { return ch == '#'; });  ///< move until next token

    it = end;

    /// Trim trailing whitespaces
    auto text = std::string_view(begin, end);
    end       = std::find_if(text.crbegin(), text.crend(), [](char const ch) { return !isspace(static_cast<unsigned char>(ch)); }).base();

    return std::string_view(text.begin(), end);  ///< We must use text.begin() because end is now belongs to text
}

/// @brief Read token with all related data
/// @param[in] str Text buffer to read from
/// @param[in,out] it An iterator pointing to place where reading needs to be started,
///                   will be set to the character right after the text that was read from str
/// @return Token, id (questID or textID) and text (quest title for the QUEST token, and text for the TEXT token)
/// @note If there is whitespaces in the beginning, they will be trimmed before reading. Trailing whitespaces will be
/// trimmed too.
std::tuple<Token, std::string_view, std::string_view> ReadFullToken(std::string_view const& str, std::string_view::const_iterator& it)
{
    if (str.empty() || it == str.cend()) return {Token::Invalid, "", ""};

    auto const tokenString = ReadWord(str, it);
    auto const token       = kTokenMap.contains(tokenString) ? kTokenMap.at(tokenString) : Token::Unknown;

    auto const id   = ReadWord(str, it);
    auto const text = ReadText(str, it);

    return {token, id, text};
};

/// @brief Read UserDataID, that is placed in special pattern
/// @param[in] str Text buffer to read from
/// @param[in,out] it An iterator pointing to place where reading needs to be started,
///                   will be set to the character right after the text that was read from str
/// @return First UserDataID int str and iterator pointing to pattern start
/// @note Resulted UserDataID will not include patter itself - only text between brackets, but iterator 'it' will be set
/// to symbol after closing bracket
std::pair<std::string_view, std::string_view::const_iterator>
ReadUserDataID(std::string_view const& str, std::string_view::const_iterator& it)
{
    auto const patternStart = std::find_if(it, str.cend(), [](char const ch) { return ch == '@'; });

    it = patternStart;
    if (patternStart == str.cend()) return {"", patternStart};

    auto textStart = std::next(patternStart);
    if (textStart == str.cend() || *textStart != '<')  ///< There must be '<' after '@'
        return {"", patternStart};

    textStart          = std::next(textStart);  ///< We need to move to an actual text after '<'
    auto const textEnd = std::find_if(textStart, str.cend(), [](char const ch) { return ch == '>'; });

    if (textEnd == str.cend()) {
        spdlog::error("Quest log file parse error: the end of pattern is not found");
        return {"", patternStart};
    }

    it = std::next(textEnd);

    return {std::string_view(textStart, textEnd), patternStart};
};

}  // namespace

namespace storm
{
QuestFileReader::QuestFileReader()  = default;
QuestFileReader::~QuestFileReader() = default;

bool QuestFileReader::GetQuestTitle(std::string_view const& questId, std::string_view const& questUniqueID, std::string& buffer)
{
    if (questId.empty()) return false;

    ReadUserData(questUniqueID, -1);

    buffer.clear();

    auto const n = FindQuestByID(questId);
    if (!n.has_value()) {
        spdlog::warn("Can`t find title whith ID = %s", questId);
        return false;
    }

    auto const& quest = quests_[n.value()];
    AssembleStringToBuffer(quest.title.c_str(), buffer, questData_);
    return true;
}

std::string
QuestFileReader::GetRecordText(std::string_view const& questID, std::string_view const& textID, std::string_view const& userData)
{
    auto const questNum = FindQuestByID(questID);
    if (!questNum.has_value()) return {};

    auto&      quest   = quests_[questNum.value()];
    auto const textNum = FindTextByID(quest, textID);
    if (!textNum.has_value()) return {};

    curQuestID_ = "";
    questData_.clear();
    FillUserDataList((char*)userData.data(), questData_);

    std::string buffer;
    auto const& unwrappedText = quest.texts[textNum.value()];
    AssembleStringToBuffer(unwrappedText.str.c_str(), buffer, questData_);

    return buffer;
}

bool QuestFileReader::AssembleStringToBuffer(std::string_view const& src, std::string& buffer, std::vector<UserData> const& userData)
{
    if (src.empty()) return false;

    buffer.clear();

    for (auto it = src.cbegin(), textStart = src.cbegin(); it != src.cend();) {
        auto [insertID, patternStart] = ReadUserDataID(src, it);
        buffer.insert(buffer.cend(), textStart, patternStart);  ///< Insert all text before pattern into a buffer

        textStart = it;

        if (patternStart == it)  ///< Found an incorrect pattern or EOF
            continue;

        std::string insertStr = GetInsertStringByID(insertID, userData);
        buffer += insertStr;
    }

    buffer.push_back(0);  ///< Workaround for methods that work with raw char* buffer

    return true;
}

std::string QuestFileReader::GetInsertStringByID(std::string_view const& id, std::vector<UserData> const& userData)
{
    if (id.empty()) return {};

    for (auto& data: userData)
        if (data.id == id) return data.str;

    return {};
}

void QuestFileReader::ReadUserData(std::string_view const& questID, int32_t recordIndex)
{
    if (curQuestID_ == questID) return;  // already set for this quest

    curQuestID_ = questID;
    questData_.clear();

    if (questID.empty()) return;

    auto* vdata = core.Event("evntQuestUserData", "sl", questID.data(), recordIndex);
    if (!vdata) return;

    auto* const str = vdata->GetString();
    if (!str || str[0] == 0) return;

    FillUserDataList(str, questData_);
}

void QuestFileReader::FillUserDataList(std::string_view const& strData, std::vector<UserData>& userData)
{
    if (strData.empty()) return;

    std::string_view currentID = "";
    for (auto it = strData.cbegin(), textStart = strData.cbegin(); it != strData.cend();) {
        auto [userDataID, patternStart] = ReadUserDataID(strData, it);

        if (!currentID.empty()) userData.emplace_back(std::string(currentID), std::string(textStart, patternStart));

        currentID = userDataID;
        textStart = it;  ///< 'it' points to symbol right after pattern, here is the place where text starts
    }
}

void QuestFileReader::SetQuestTextFileName(std::string_view const& fileName)
{
    if (fileName.empty()) return;

    // check for already included:
    for (auto& questFileName: questFileNames_)
        if (questFileName == fileName) return;

    questFileNames_.push_back(std::string(fileName));

    /// Open file
    auto fileS = std::ifstream(fileName.data(), std::ios::binary);
    if (!fileS.is_open()) {
        core.Trace("WARNING! Can`t open quest log file %s", std::string(fileName).c_str());
        return;
    }

    /// Obtain file size
    uint32_t const filesize = fio->file_size(fileName.data());
    if (filesize == 0) {
        core.Trace("Empty quest log file %s", std::string(fileName).c_str());
        return;
    }

    /// Create a buffer
    std::string buffer(filesize, '\0');
    Assert(!buffer.empty());

    // Read file content into buffer
    fileS.read(buffer.data(), filesize);

    AddQuestsFromBuffer(std::string_view(buffer));
}

void QuestFileReader::AddQuestsFromBuffer(std::string_view const& srcBuffer)
{
    if (srcBuffer.empty()) return;

    std::string_view curQuestId;
    for (auto it = srcBuffer.cbegin(); it != srcBuffer.cend();) {
        auto [token, id, text] = ReadFullToken(srcBuffer, it);
        if (token == Token::Quest) {
            AddQuestToList(id, text);
            curQuestId = id;
        } else if (token == Token::Text) {
            AddTextToQuest(curQuestId, id, text);
        }
    }
}

void QuestFileReader::AddQuestToList(std::string_view const& questID, std::string_view const& titleText)
{
    if (questID.empty()) return;

    auto q = FindQuestByID(questID);
    if (!q.has_value()) {
        QuestDescribe descr;
        descr.questID = questID;
        descr.title   = titleText;
        quests_.push_back(descr);
    } else {
        spdlog::warn("Quest with id \"%s\" is already in list", questID);
    }
}

void QuestFileReader::AddTextToQuest(std::string_view const& questID, std::string_view const& textID, std::string_view const& questText)
{
    if (questID.empty() || textID.empty()) return;

    auto q = FindQuestByID(questID);
    if (!q.has_value()) {
        spdlog::warn("Quest with id \"%s\" is not found in list", questID);
        return;
    }

    auto&      quest = quests_[q.value()];
    auto const t     = FindTextByID(quest, textID);
    if (!t.has_value()) {
        QuestDescribe::TextDescribe descr;
        descr.id  = textID;
        descr.str = questText;
        quest.texts.push_back(descr);
    }
}

std::optional<size_t> QuestFileReader::FindQuestByID(std::string_view const& questID)
{
    if (questID.empty()) return std::nullopt;

    for (size_t i = 0; i < quests_.size(); ++i)
        if (quests_[i].questID == questID) return i;

    return std::nullopt;
}

std::optional<size_t> QuestFileReader::FindTextByID(QuestFileReader::QuestDescribe& nQuest, std::string_view const& textID)
{
    if (textID.empty()) return std::nullopt;

    auto& questTexts = nQuest.texts;
    for (size_t i = 0; i < questTexts.size(); ++i)
        if (questTexts[i].id == textID) return i;

    return std::nullopt;
}
}  // namespace storm
