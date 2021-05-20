cmake_minimum_required(VERSION 3.14)
project(netspeak4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-m64")
set(CMAKE_EXE_LINKER_FLAGS "-m64")

set(CMAKE_CXX_COMPILER             "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS                "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG          "-g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")


include_directories(
    "src"
    "/usr/include/antlr4-runtime"
)

set(NETSPEAK_SOURCES
"src/antlr4/parse"
"src/antlr4/QueryErrorHandler"

"src/antlr4/generated/QueryBaseListener"
"src/antlr4/generated/QueryBaseVisitor"
"src/antlr4/generated/QueryLexer"
"src/antlr4/generated/QueryListener"
"src/antlr4/generated/QueryParser"
"src/antlr4/generated/QueryVisitor"

"src/cli/BuildCommand"
"src/cli/Command"
"src/cli/Commands"
"src/cli/logging"
"src/cli/ProxyCommand"
"src/cli/RegexCommand"
"src/cli/ServeCommand"
"src/cli/ShellCommand"
"src/cli/StressCommand"
"src/cli/util"

"src/netspeak/Configuration"
"src/netspeak/Dictionaries"
"src/netspeak/error"
"src/netspeak/indexing"
"src/netspeak/Netspeak"
"src/netspeak/PhraseCorpus"
"src/netspeak/PhraseDictionary"
"src/netspeak/PhraseFileParser"
"src/netspeak/PhraseFileReader"
"src/netspeak/Properties"
"src/netspeak/QueryNormalizer"
"src/netspeak/QueryProcessor"
"src/netspeak/RetrievalStrategy"
"src/netspeak/RetrievalStrategy3"

"src/netspeak/bighashmap/BigHashMap"
"src/netspeak/bighashmap/Builder"
"src/netspeak/bighashmap/CmphMap"
"src/netspeak/bighashmap/ExternalMap"
"src/netspeak/bighashmap/InternalMap"

"src/netspeak/invertedindex/ByteBuffer"
"src/netspeak/invertedindex/Configuration"
"src/netspeak/invertedindex/Indexer"
"src/netspeak/invertedindex/IndexStrategy"
"src/netspeak/invertedindex/InvertedFileReader"
"src/netspeak/invertedindex/InvertedFileWriter"
"src/netspeak/invertedindex/Iterator"
"src/netspeak/invertedindex/ManagedIndexer"
"src/netspeak/invertedindex/Postlist"
"src/netspeak/invertedindex/PostlistBuilder"
"src/netspeak/invertedindex/PostlistReader"
"src/netspeak/invertedindex/PostlistSorter"
"src/netspeak/invertedindex/Properties"
"src/netspeak/invertedindex/RawIndexer"
"src/netspeak/invertedindex/RawPostlist"
"src/netspeak/invertedindex/RawSearcher"
"src/netspeak/invertedindex/Record"
"src/netspeak/invertedindex/RecordReader"
"src/netspeak/invertedindex/Searcher"
"src/netspeak/invertedindex/SortedInput"
"src/netspeak/invertedindex/StorageReader"
"src/netspeak/invertedindex/StorageWriter"
"src/netspeak/invertedindex/UnsortedInput"

"src/netspeak/model/LengthRange"
"src/netspeak/model/NormQuery"
"src/netspeak/model/Phrase"
"src/netspeak/model/Query"
"src/netspeak/model/QuerySyntax"
"src/netspeak/model/RawPhraseResult"
"src/netspeak/model/RawRefResult"
"src/netspeak/model/RawResult"
"src/netspeak/model/SearchOptions"
"src/netspeak/model/SearchResult"
"src/netspeak/model/SimpleQuery"
"src/netspeak/model/typedefs"
"src/netspeak/model/Words"

"src/netspeak/regex/DefaultRegexIndex"
"src/netspeak/regex/parsers"
"src/netspeak/regex/RegexIndex"
"src/netspeak/regex/RegexQuery"

"src/netspeak/service/LoadBalanceProxy"
"src/netspeak/service/NetspeakService.grpc.pb"
"src/netspeak/service/NetspeakService.pb"
"src/netspeak/service/RequestLogger"
"src/netspeak/service/tracking"
"src/netspeak/service/UniqueMap"

"src/netspeak/util/ChainCutter"
"src/netspeak/util/check"
"src/netspeak/util/checksum"
"src/netspeak/util/Config"
"src/netspeak/util/conversion"
"src/netspeak/util/exception"
"src/netspeak/util/FileDescriptor"
"src/netspeak/util/glob"
"src/netspeak/util/JsonWriter"
"src/netspeak/util/LfuCache"
"src/netspeak/util/logging"
"src/netspeak/util/Math"
"src/netspeak/util/memory"
"src/netspeak/util/Mut"
"src/netspeak/util/PropertiesFormat"
"src/netspeak/util/service"
"src/netspeak/util/string"
"src/netspeak/util/systemio"
"src/netspeak/util/traceable_error"
"src/netspeak/util/Vec"

"src/netspeak/value/big_string"
"src/netspeak/value/big_string_traits"
"src/netspeak/value/pair"
"src/netspeak/value/pair_traits"
"src/netspeak/value/quadruple"
"src/netspeak/value/quadruple_traits"
"src/netspeak/value/quintuple"
"src/netspeak/value/quintuple_traits"
"src/netspeak/value/sextuple"
"src/netspeak/value/sextuple_traits"
"src/netspeak/value/string_traits"
"src/netspeak/value/triple"
"src/netspeak/value/triple_traits"
"src/netspeak/value/value_traits"
)
set(NETSPEAK_LINK_LIBS
dl
z
rt
pthread

# cmph (C Minimal Perfect Hashing)
-l:libcmph.a

# boost
-l:libboost_date_time.a
-l:libboost_filesystem.a
-l:libboost_system.a
-l:libboost_program_options.a
-l:libboost_regex.a
-l:libboost_stacktrace_basic.a
-l:libboost_stacktrace_backtrace.a
-l:libboost_stacktrace_addr2line.a

# antlr4
-l:libantlr4-runtime.a

# protobuf
-l:libprotobuf.a

# grpc
-l:libgrpc++.a
-l:libgrpc_unsecure.a
-l:libcares.a
-l:libgrpc.a
-l:libgpr.a
-l:libabsl_bad_optional_access.a
-l:libabsl_base.a
-l:libabsl_log_severity.a
-l:libabsl_spinlock_wait.a
-l:libabsl_str_format_internal.a
-l:libabsl_strings.a
-l:libabsl_raw_logging_internal.a
-l:libabsl_strings_internal.a
-l:libabsl_int128.a
-l:libabsl_throw_delegate.a
-l:libaddress_sorting.a
-l:libupb.a
)

set(NETSPEAK_TEST_SOURCES "${NETSPEAK_SOURCES}"
"test/netspeak/ManagedDirectory"
"test/netspeak/paths"
"test/netspeak/test_ChainCutter"
"test/netspeak/test_LfuCache"
"test/netspeak/test_Netspeak"
"test/netspeak/test_normalization"
"test/netspeak/test_parse"
"test/netspeak/test_phrase"
"test/netspeak/test_PhraseCorpus"
"test/netspeak/test_PropertiesFormat"
"test/netspeak/test_QueryParser"
"test/netspeak/test_regex"

"test/netspeak/bighashmap/test_big_hash_map"
"test/netspeak/bighashmap/test_value_traits"

"test/netspeak/invertedindex/test_Indexer"
"test/netspeak/invertedindex/test_InvertedFileReader"
"test/netspeak/invertedindex/test_ManagedIndexer"
"test/netspeak/invertedindex/test_Postlist"
"test/netspeak/invertedindex/test_PostlistReader"
"test/netspeak/invertedindex/test_PostlistSorter"
"test/netspeak/invertedindex/test_Record"
)
set(NETSPEAK_TEST_LINK_LIBS "${NETSPEAK_LINK_LIBS}")


add_executable(netspeak4 src/CommandLineTool.cpp)
target_sources(netspeak4 PRIVATE "${NETSPEAK_SOURCES}")
target_link_libraries(netspeak4 "${NETSPEAK_LINK_LIBS}")

add_executable(netspeak4-test test/netspeak/runner.cpp)
target_sources(netspeak4-test PRIVATE "${NETSPEAK_TEST_SOURCES}")
target_link_libraries(netspeak4-test "${NETSPEAK_TEST_LINK_LIBS}")
