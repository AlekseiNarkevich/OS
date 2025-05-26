#include <gtest/gtest.h>
#include "structs.h"
#include <fstream>
#include <windows.h>
#include <climits>
#include <cstring>
TEST(SharedDataTest, StructureValidation) 
{
    ASSERT_EQ(sizeof(SharedData), 28) << "Некорректный размер структуры (4 + 4 + 20)";
    SharedData data{};
    const char* long_msg = "This is a very long message over 20 characters";
    strncpy_s(data.message, long_msg, sizeof(data.message) - 1);
    data.message[sizeof(data.message) - 1] = '\0';
    ASSERT_EQ(strlen(data.message), 19) << "Сообщение должно обрезаться до 19 символов";
}
class SenderTest : public ::testing::Test 
{
protected:
    const std::string test_file = "test_sender.bin";
    const int thread_id = 1;
    void SetUp() override 
    {
        std::ofstream file(test_file, std::ios::binary);
        SharedData data = { -1, -1, {} };
        file.write(reinterpret_cast<char*>(&data), sizeof(data));
    }
    void TearDown() override
    {
        remove(test_file.c_str());
    }
};
TEST_F(SenderTest, MessageWritingLogic)
{
    std::fstream file(test_file, std::ios::binary | std::ios::in | std::ios::out);
    std::vector<SharedData> buffer;
    SharedData temp;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(temp)))
    {
        buffer.push_back(temp);
    }
    file.clear();
    file.seekp(0);
    ASSERT_FALSE(buffer.empty()) << "Файл не содержит данных для модификации";
    bool isModified = false;
    for (auto& item : buffer) 
    {
        if (item.messageID == -1) 
        {
            strncpy_s(item.message, "Test123", sizeof(item.message) - 1);
            item.messageID = 0;
            item.writerID = thread_id;
            isModified = true;
            break;
        }
    }
    ASSERT_TRUE(isModified) << "Запись с messageID = -1 не найдена";
    for (const auto& item : buffer)
    {
        file.write(reinterpret_cast<const char*>(&item), sizeof(item));
    }
    file.close();
    std::ifstream in(test_file, std::ios::binary);
    SharedData result;
    in.read(reinterpret_cast<char*>(&result), sizeof(result));
    ASSERT_EQ(result.messageID, 0) << "messageID не обновлён";
    ASSERT_EQ(result.writerID, thread_id) << "writerID не совпадает";
    ASSERT_STREQ(result.message, "Test123") << "Сообщение записано неверно";
}
TEST(ReceiverTest, FileInitialization)
{
    const std::string test_file = "test_receiver.bin";
    const int capacity = 5;
    {
        std::ofstream file(test_file, std::ios::binary);
        for (int i = 0; i < capacity; ++i) 
        {
            SharedData data = {-1, -1, {'a'}};
            file.write(reinterpret_cast<char*>(&data), sizeof(data));
        }
    }
    std::ifstream in(test_file, std::ios::binary);
    SharedData data;
    int count = 0;
    while (in.read(reinterpret_cast<char*>(&data), sizeof(data))) 
    {
        ASSERT_EQ(data.messageID, -1);
        count++;
    }
    ASSERT_EQ(count, capacity);
    remove(test_file.c_str());
}