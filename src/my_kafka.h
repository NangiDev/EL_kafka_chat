#pragma once

#pragma warning( push )
#pragma warning(disable : 4251 4081 4005 4275);
#include <stdexcept>
#include <iostream>
#include <boost/program_options.hpp>
#include "cppkafka/producer.h"
#include "cppkafka/consumer.h"
#include "cppkafka/configuration.h"
#pragma warning( pop )

#include <random>

using std::string;
using std::exception;
using std::getline;
using std::cin;
using std::cout;
using std::endl;

using cppkafka::Consumer;
using cppkafka::Producer;
using cppkafka::Configuration;
using cppkafka::Topic;
using cppkafka::Message;
using cppkafka::MessageBuilder;

namespace po = boost::program_options;

std::random_device gRnd;
std::mt19937_64 gen;

inline uint64_t rnd_num() {
    gen = std::mt19937_64(gRnd());
    return gen();
}

string ROOM_PREFIX = "room_";

string brokers = "localhost:9093";
string topic_name = "topic";
string group_id = std::to_string(rnd_num());

string getCurrentTopic() {
    return ROOM_PREFIX + topic_name;
}

namespace Publisher {

    MessageBuilder builder(getCurrentTopic());
    Configuration config = {
        { "metadata.broker.list", brokers }
    };
    Producer producer(config);

    void publishMessage(string message) {
        if (message.size() < 1)
            return;

        builder.payload(message);

        builder.key(group_id);

        producer.produce(builder);
        producer.flush();
    }
}


namespace Reciever {

    Configuration config = {
        { "metadata.broker.list", brokers },
        { "group.id", group_id },
        // Disable auto commit
        { "enable.auto.commit", true },
        { "auto.offset.reset", "earliest" }
    };

    Consumer consumer(config);

    int delay, resetDelay = 0;
    bool running = true;

    std::vector<cppkafka::TopicMetadata>* roomList;
    std::vector<std::string>* historyList;

    void subscribe() {
        // Subscribe to the topic
        consumer.subscribe({ getCurrentTopic() });
    }

    std::vector<cppkafka::TopicMetadata> getTopics() {
        return consumer.get_metadata().get_topics();
    }

    void consumeMessage() {
        // Try to consume a message
        delay--;

        if (delay > 0) {
            //cout << delay << endl;
            return;
        }
        
        *roomList = getTopics();
        
        try
        {
            std::vector<cppkafka::Message> list = consumer.poll_batch(100);

            for (int i = 0; i < list.size(); i++) {

                cppkafka::Message* msg = &list[i];

                if (msg) {
                    // If we managed to get a message
                    if (msg->get_error()) {
                        // Ignore EOF notifications from rdkafka
                        if (!msg->is_eof()) {
                            cout << "[+] Received error notification: " << msg->get_error() << endl;
                        }
                    }
                    else {
                        string message = "";
                        if (group_id == msg->get_key()) {
                            message.append("YOU -> ");
                        }
                        else {
                            message.append(msg->get_key());
                            message.append(" -> ");
                        }

                        message.append(msg->get_payload().operator std::string());

                        historyList->push_back(message);

                        // Now commit the message
                        consumer.commit(*msg);
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            cout << e.what() << endl;
        }

        delay = resetDelay;
    }

    void start(std::vector<cppkafka::TopicMetadata> *roomList_, std::vector<std::string> *historyList_) {
        roomList = roomList_;
        historyList = historyList_;

        running = true;
        Reciever::subscribe();

        while (running) {
            consumeMessage();
        }
    }

    void stop() {
        running = false;
    }
}