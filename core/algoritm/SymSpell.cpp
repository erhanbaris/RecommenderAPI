#include <core/algoritm/SymSpell.h>
#include <core/Utils.h>

#define getHastCode(term) hash<string>()(term)

using namespace std;
using namespace core::algoritm;

namespace {

    struct Xgreater1 {
        bool operator()(const FindedItem &lx, const FindedItem &rx) const {
            return lx.distance > rx.distance;
        }
    };

    struct Xgreater2 {
        bool operator()(const FindedItem &lx, const FindedItem &rx) const {
            auto cmpForLx = 2 * (lx.distance - rx.distance);
            auto cmpForRx = 2 * (rx.distance - lx.distance);
            return cmpForLx > cmpForRx;
        }
    };

    size_t Verbose = 0;
    size_t MaxDistance = 2;
    size_t maxlength = 1000;
    CUSTOM_MAP<size_t, dictionaryItemContainer> dictionary;
    vector<string> wordlist;
}

SymSpell::SymSpell() {
    setlocale(LC_ALL, "");
#ifdef USE_GOOGLE_DENSE_HASH_MAP
    dictionary.set_empty_key(0);
#endif
}

#ifdef IO_OPERATIONS

void SymSpell::Save(string filePath)
{
    std::ofstream fileStream(filePath, ios::binary);

    if (fileStream.good())
    {
#ifdef USE_GOOGLE_DENSE_HASH_MAP
        std::unordered_map<size_t, dictionaryItemContainer> tmpDict(dictionary.begin(), dictionary.end()); // should be undered_map
#endif

        msgpack::packer<std::ofstream> packer(&fileStream);

        packer.pack(this->verbose);
        packer.pack(this->editDistanceMax);
        packer.pack(this->maxlength);

#ifdef USE_GOOGLE_DENSE_HASH_MAP
        packer.pack(tmpDict);
#else
        packer.pack(dictionary);
#endif
        packer.pack(this->wordlist);
    }

    fileStream.close();
}

void SymSpell::Load(string filePath)
{
    std::ifstream fileStream(filePath, ios::binary);

    if (fileStream.good())
    {
        std::string str((std::istreambuf_iterator<char>(fileStream)), (std::istreambuf_iterator<char>()));

        msgpack::unpacker packer;

        packer.reserve_buffer(str.length());
        memcpy(packer.buffer(), str.data(), str.length());
        packer.buffer_consumed(str.length());

        msgpack::object_handle handler;
        packer.next(handler);

        handler.get().convert(this->verbose);
        packer.next(handler);

        handler.get().convert(this->editDistanceMax);
        packer.next(handler);

        handler.get().convert(this->maxlength);
        packer.next(handler);

#ifdef USE_GOOGLE_DENSE_HASH_MAP
        std::unordered_map<size_t, dictionaryItemContainer> tmpDict;
        handler.get().convert(tmpDict);
        this->dictionary.insert(tmpDict.begin(), tmpDict.end());
#else
        handler.get().convert(this->dictionary);
#endif
        packer.next(handler);
        handler.get().convert(this->wordlist);

    }

    fileStream.close();
}

#endif

bool SymSpell::CreateDictionaryEntry(string key, PRODUCT_TYPE id) {
    bool result = false;
    dictionaryItemContainer value;

    auto dictionaryEnd = dictionary.end(); // for performance
    auto valueo = dictionary.find(getHastCode(key));
    if (valueo != dictionaryEnd) {
        value = valueo->second;

        if (valueo->second.itemType == ItemType::INTEGER) {
            value.itemType = ItemType::DICT;
            value.dictValue = std::make_shared<dictionaryItem>();
            value.dictValue->suggestions.push_back(valueo->second.intValue);
        } else
            value = valueo->second;

        if (value.dictValue->count < INT_MAX)
            ++(value.dictValue->count);
    } else if (wordlist.size() < INT_MAX) {
        value.itemType = ItemType::DICT;
        value.dictValue = std::make_shared<dictionaryItem>();
        ++(value.dictValue->count);
        string mapKey = key;
        dictionary[getHastCode(mapKey)] = value;
        dictionaryEnd = dictionary.end(); // for performance

        if (key.size() > maxlength)
            maxlength = key.size();
    }
    
    value.Id->insert(id);

    wordlist.push_back(key);
    size_t keyint = wordlist.size() - 1;

    result = true;

    auto deleted = CUSTOM_SET<string>();
#ifdef USE_GOOGLE_DENSE_HASH_MAP
    deleted.set_empty_key("");
#endif

    if (core::isInteger(key))
        return result;

    Edits(key, deleted);

    for (string del : deleted) {
        auto value2 = dictionary.find(getHastCode(del));
        if (value2 != dictionaryEnd) {
            if (value2->second.itemType == ItemType::INTEGER) {
                value2->second.itemType = ItemType::DICT;
                value2->second.dictValue = std::make_shared<dictionaryItem>();
                value2->second.dictValue->suggestions.push_back(value2->second.intValue);
                dictionary[getHastCode(del)].dictValue = value2->second.dictValue;

                if (std::find(value2->second.dictValue->suggestions.begin(), value2->second.dictValue->suggestions.end(), keyint) == value2->second.dictValue->suggestions.end())
                    AddLowestDistance(value2->second.dictValue, key, keyint, del);
            } else if (std::find(value2->second.dictValue->suggestions.begin(), value2->second.dictValue->suggestions.end(), keyint) == value2->second.dictValue->suggestions.end())
                AddLowestDistance(value2->second.dictValue, key, keyint, del);
            value2->second.Id->insert(id);
        } else {
            dictionaryItemContainer tmp;
            tmp.itemType = ItemType::INTEGER;
            tmp.intValue = keyint;
            tmp.Id->insert(id);

            dictionary[getHastCode(del)] = tmp;
            dictionaryEnd = dictionary.end();
        }
    }
    return result;
}

CUSTOM_MAP<PRODUCT_TYPE, FindedItem> SymSpell::Find(string input) const {
    CUSTOM_MAP<PRODUCT_TYPE, FindedItem> suggestions;

#ifdef ENABLE_TEST
    using namespace std::chrono;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (size_t i = 0; i < 100000; ++i)
    {
        Lookup(input, editDistanceMax);
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

    std::cout << "It took me " << time_span.count() << " seconds.";
    std::cout << std::endl;
#endif
    //todo: order results
    suggestions = Lookup(input, MaxDistance);
    return suggestions;

}

vector<string> SymSpell::parseWords(string text) const {
    vector<string> returnData;

    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    std::regex word_regex("[^\\W\\d_]+");
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), word_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        returnData.push_back(match.str());
    }

    return returnData;
}

void SymSpell::AddLowestDistance(shared_ptr<dictionaryItem> const &item, string suggestion, size_t suggestionint,
                                 string del) {
    if ((Verbose < 2) && (item->suggestions.size() > 0) &&
        (wordlist[item->suggestions[0]].size() - del.size() > suggestion.size() - del.size()))
        item->suggestions.clear();

    if ((Verbose == 2) || (item->suggestions.size() == 0) ||
        (wordlist[item->suggestions[0]].size() - del.size() >= suggestion.size() - del.size()))
        item->suggestions.push_back(suggestionint);
}

void SymSpell::Edits(string word, CUSTOM_SET<string> &deletes) const {
    CUSTOM_MAP<size_t, const char *> queue;
#ifdef USE_GOOGLE_DENSE_HASH_MAP
    queue.set_empty_key(0);
#endif

    auto wordLength = word.size();
    char *copyWord = new char[wordLength + 1];
    strcpy(copyWord, word.c_str());
    copyWord[wordLength] = '\0';

    queue[getHastCode(word)] = copyWord;

    for (size_t d = 0; d < MaxDistance; ++d) {
        CUSTOM_MAP<size_t, const char *> tempQueue;
        auto tempQueueEnd = tempQueue.end();
#ifdef USE_GOOGLE_DENSE_HASH_MAP
        tempQueue.set_empty_key(0);
#endif

        for (auto item : queue) {
            if (strlen(item.second)) {
                auto itemLength = strlen(item.second);

                for (size_t i = 0; i < itemLength; ++i) {
                    // For Performance ->
                    char *del = new char[itemLength + 1];

                    strcpy(del, item.second);
                    del[itemLength] = '\0';
                    size_t k = i;
                    size_t len = strlen(item.second);
                    for (; k < len - 1; k++)
                        del[k] = item.second[k + 1];
                    del[k] = '\0';
                    // <- For Performance

                    if (!deletes.count(del))
                        deletes.insert((basic_string<char, char_traits<char>, allocator<char>> &&) del);

                    if (tempQueue.find(getHastCode(del)) == tempQueueEnd) {
                        tempQueue[getHastCode(del)] = del;
                        tempQueueEnd = tempQueue.end();
                    }
                }
            }
        }

        for (auto item : queue)
            delete item.second;

        queue = tempQueue;
    }
}

CUSTOM_MAP<PRODUCT_TYPE, FindedItem> SymSpell::Lookup(string input, size_t editDistanceMax) const {
    if (input.size() - editDistanceMax > maxlength)
        return CUSTOM_MAP<PRODUCT_TYPE, FindedItem>();

    vector<string> candidates;
    candidates.reserve(2048);
    CUSTOM_SET<size_t> hashset1;
#ifdef USE_GOOGLE_DENSE_HASH_MAP
    hashset1.set_empty_key(0);
#endif

    CUSTOM_MAP<PRODUCT_TYPE, FindedItem> suggestions;
    INIT_MAP(suggestions, 0, -1);
    CUSTOM_SET<size_t> hashset2;
#ifdef USE_GOOGLE_DENSE_HASH_MAP
    hashset2.set_empty_key(0);
#endif


    bool isInputInteger = core::isInteger(input);

    candidates.push_back(input);
    auto dictionaryEnd = dictionary.end();

    size_t candidatesIndexer = 0; // for performance
    while ((candidates.size() - candidatesIndexer) > 0) {
        string candidate = candidates[candidatesIndexer];
        size_t candidateSize = candidate.size(); // for performance
        ++candidatesIndexer;

        if ((Verbose < 2) && (suggestions.size() > 0) &&
            (input.size() - candidateSize > suggestions.begin()->second.distance))
            break;

        auto valueo = dictionary.find(getHastCode(candidate));

        //read candidate entry from dictionary
        if (valueo != dictionaryEnd) {
            if (valueo->second.itemType == ItemType::INTEGER) {
                valueo->second.itemType = ItemType::DICT;
                valueo->second.dictValue = std::make_shared<dictionaryItem>();
                valueo->second.dictValue->suggestions.push_back(valueo->second.intValue);
            }


            if (valueo->second.itemType == ItemType::DICT &&
                valueo->second.dictValue->count > 0 &&
                hashset2.insert(getHastCode(candidate)).second) {
                //add correct dictionary term term to suggestion list

                auto idEnd = valueo->second.Id->end();
                for (auto it = valueo->second.Id->begin(); it != idEnd; ++it) {
                    //todo: fix this code for duplicate items
                    FindedItem si;
                    si.term = candidate;
                    si.distance = (unsigned short) (input.size() - candidateSize);
                    si.productId = *it;
                    suggestions[si.productId] = si;
                }

                //early termination
                if ((Verbose < 2) && (input.size() - candidateSize == 0))
                    break;
            }

            for (size_t suggestionint : valueo->second.dictValue->suggestions) {
                //save some time
                //skipping double items early: different deletes of the input term can lead to the same suggestion
                //index2word
                string suggestion = wordlist[suggestionint];
                if (hashset2.insert(getHastCode(suggestion)).second) {
                    size_t distance = 0;
                    if (suggestion != input) {
                        if (suggestion.size() == candidateSize) distance = input.size() - candidateSize;
                        else if (input.size() == candidateSize)
                            distance = suggestion.size() - candidateSize;
                        else {
                            size_t ii = 0;
                            size_t jj = 0;
                            while ((ii < suggestion.size()) && (ii < input.size()) &&
                                   (suggestion[ii] == input[ii]))
                                ++ii;

                            while ((jj < suggestion.size() - ii) && (jj < input.size() - ii) &&
                                   (suggestion[suggestion.size() - jj - 1] == input[input.size() - jj - 1]))
                                ++jj;
                            if ((ii > 0) || (jj > 0))
                                distance = DamerauLevenshteinDistance(
                                        suggestion.substr(ii, suggestion.size() - ii - jj),
                                        input.substr(ii, input.size() - ii - jj));
                            else
                                distance = DamerauLevenshteinDistance(suggestion, input);

                        }
                    }

                    if ((Verbose < 2) && (suggestions.size() > 0) && (suggestions.begin()->second.distance > distance))
                        suggestions.clear();

                    if ((Verbose < 2) && (suggestions.size() > 0) && (distance > suggestions.begin()->second.distance))
                        continue;

                    if (distance <= editDistanceMax) {
                        auto value2 = dictionary.find(getHastCode(suggestion));

                        if (value2 != dictionaryEnd) {
                            auto idEnd = valueo->second.Id->end();
                            for (auto it = valueo->second.Id->begin(); it != idEnd; ++it) {
                                FindedItem si;
                                si.term = candidate;
                                si.distance = (unsigned short) (input.size() - candidateSize);
                                si.productId = *it;
                                suggestions[si.productId] = si;
                            }
                        }
                    }
                }
            }
        }
        else if (isInputInteger)
            return CUSTOM_MAP<PRODUCT_TYPE, FindedItem>();

        if (input.size() - candidateSize < editDistanceMax) {
            if ((Verbose < 2) && (suggestions.size() > 0) &&
                (input.size() - candidateSize >= suggestions.begin()->second.distance))
                continue;

            for (size_t i = 0; i < candidateSize; ++i) {
                string wordClone = candidate;
                string &del = wordClone.erase(i, 1);
                if (hashset1.insert(getHastCode(del)).second)
                    candidates.push_back(del);
            }
        }
    }//end while

    /*sort:
    if (Verbose < 2)
        sort(suggestions.begin(), suggestions.end(), Xgreater1());
    else
        sort(suggestions.begin(), suggestions.end(), Xgreater2());*/

    return suggestions;
}


size_t SymSpell::DamerauLevenshteinDistance(const std::string &s1, const std::string &s2) {
    const size_t m(s1.size());
    const size_t n(s2.size());

    if (m == 0) return n;
    if (n == 0) return m;

    size_t *costs = new size_t[n + 1];

    for (size_t k = 0; k <= n; ++k) costs[k] = k;

    size_t i = 0;
    auto s1End = s1.end();
    auto s2End = s2.end();
    for (std::string::const_iterator it1 = s1.begin(); it1 != s1End; ++it1, ++i) {
        costs[0] = i + 1;
        size_t corner = i;

        size_t j = 0;
        for (std::string::const_iterator it2 = s2.begin(); it2 != s2End; ++it2, ++j) {
            size_t upper = costs[j + 1];
            if (*it1 == *it2) {
                costs[j + 1] = corner;
            } else {
                size_t t(upper < corner ? upper : corner);
                costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
            }

            corner = upper;
        }
    }

    size_t result = costs[n];
    delete[] costs;

    return result;
}