#include <gtest/gtest.h>
#include "structs.h"
#include <fstream>
#include <cstring>
#include <vector>
TEST(SharedDataTest, StructureValidation)
{
    ASSERT_EQ(sizeof(SharedData), 28) << "Некорректный размер структуры (4 + 4 + 20)";
    SharedData data{};
    const char* long_msg = "This is a very long message over 20 characters";
    strncpy_s(data.message, long_msg, sizeof(data.message) - 1);
    data.message[sizeof(data.message) - 1] = '\0';
    ASSERT_EQ(strlen(data.message), 19);
}
class SenderTest : public ::testing::Test
{
protected:
    const std::string test_file = "test_sender.bin";
    const int thread_id = 1;
    std::vector<SharedData> buffer;
    void SetUp() override 
    {
        buffer.push_back({ -1, -1, {} });
        SaveToFile();
    }
    void SaveToFile()
    {
        std::ofstream file(test_file, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(file.is_open()) << "Не удалось открыть файл для записи";
        for (const auto& item : buffer)
        {
            file.write(reinterpret_cast<const char*>(&item), sizeof(item));
        }
        file.close();
    }
    void LoadFromFile()
    {
        buffer.clear();
        std::ifstream file(test_file, std::ios::binary);
        ASSERT_TRUE(file.is_open()) << "Не удалось открыть файл для чтения";
        SharedData temp;
        while (file.read(reinterpret_cast<char*>(&temp), sizeof(temp)))
        {
            buffer.push_back(temp);
        }
        file.close();
    }
};
TEST_F(SenderTest, MessageUpdateLogic)
{
    int lowestID = INT_MAX;
    int maxID = -1;
    for (const auto& item : buffer)
    {
        if (item.messageID < lowestID) lowestID = item.messageID;
        if (item.messageID > maxID) maxID = item.messageID;
    }
    for (auto& item : buffer)
    {
        if (item.messageID == lowestID)
        {
            strncpy_s(item.message, "Test123", sizeof(item.message) - 1);
            item.messageID = maxID + 1;
            item.writerID = thread_id;
            break;
        }
    }
    SaveToFile();
    LoadFromFile();
    ASSERT_FALSE(buffer.empty()) << "Файл пуст после записи";
    ASSERT_EQ(buffer[0].messageID, 0);
    ASSERT_EQ(buffer[0].writerID, thread_id);
    ASSERT_STREQ(buffer[0].message, "Test123");
}
TEST(ReceiverTest, FileInitialization)
{
    const std::string test_file = "test_receiver.bin";
    const int capacity = 5;
    {
        std::ofstream file(test_file, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(file.is_open()) << "Не удалось создать файл";
        for (int i = 0; i < capacity; ++i)
        {
            SharedData data = { -1, -1, {'a'} };
            file.write(reinterpret_cast<const char*>(&data), sizeof(data));
        }
        file.close();
    }
    std::ifstream in(test_file, std::ios::binary);
    ASSERT_TRUE(in.is_open()) << "Не удалось открыть файл для проверки";
    SharedData temp;
    int count = 0;
    while (in.read(reinterpret_cast<char*>(&temp), sizeof(temp)))
    {
        ASSERT_EQ(temp.messageID, -1) << "Некорректный messageID в позиции " << count;
        count++;
    }
    ASSERT_EQ(count, capacity) << "Количество записей не соответствует ожидаемому";
}
TEST(IntegrationTest, MessageOrderValidation)
{
    const std::string test_file = "fifo_test.bin";
    const int capacity = 3;
    std::vector<SharedData> test_data =
    {
        {0, 1, "First"},
        {1, 2, "Second"},
        {2, 3, "Third"}
    };
    {
        std::ofstream out(test_file, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(out.is_open()) << "Не удалось создать файл";
        for (const auto& item : test_data)
        {
            out.write(reinterpret_cast<const char*>(&item), sizeof(item));
        }
        out.close();
    }
    std::ifstream in(test_file, std::ios::binary);
    ASSERT_TRUE(in.is_open()) << "Не удалось открыть файл для чтения";
    SharedData temp;
    for (int i = 0; i < capacity; ++i) 
    {
        ASSERT_TRUE(in.read(reinterpret_cast<char*>(&temp), sizeof(temp)))
            << "Ошибка чтения записи " << i;
        ASSERT_EQ(temp.messageID, i) << "Некорректный порядок в позиции " << i;
    }
}