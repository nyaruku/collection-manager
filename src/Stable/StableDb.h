#pragma once
#include <chrono>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <zlib.h>
#include <Models/Collection.h>

namespace stable {

    inline void readBytes(std::istream& stream, void* dst, std::streamsize count) {
        if (!stream.read(static_cast<char*>(dst), count)) {
            throw std::runtime_error("Unexpected end of file");
        }
    }

    template<typename T>
    T read(std::istream& stream) {
        T value{};
        readBytes(stream, &value, sizeof(T));
        return value;
    }

    inline uint32_t readU32(std::istream& stream) { return read<uint32_t>(stream); }
    inline int32_t readI32(std::istream& stream) { return read<int32_t>(stream); }
    inline uint16_t readU16(std::istream& stream) { return read<uint16_t>(stream); }
    inline uint8_t readU8(std::istream& stream) { return read<uint8_t>(stream); }
    inline bool readBool(std::istream& stream) { return readU8(stream) != 0; }
    inline double readF64(std::istream& stream) { return read<double>(stream); }
    inline float readF32(std::istream& stream) { return read<float>(stream); }
    inline int64_t readI64(std::istream& stream) { return read<int64_t>(stream); }

    // ULEB128 used for both osu! string lengths and .NET BinaryWriter string lengths
    inline uint32_t readUleb128(std::istream& stream) {
        uint32_t result = 0;
        int shift = 0;
        uint8_t byte;
        do {
            byte = readU8(stream);
            result |= static_cast<uint32_t>(byte & 0x7F) << shift;
            shift += 7;
        } while (byte & 0x80);
        return result;
    }

    // osu! DB string: 0x00 = null/empty, 0x0b = ULEB128 length + UTF-8 bytes
    inline std::string readOsuString(std::istream& stream) {
        uint8_t indicator = readU8(stream);
        if (indicator == 0x00) {
            return {};
        }
        if (indicator != 0x0b) {
            throw std::runtime_error("Invalid osu string indicator: " + std::to_string(indicator));
        }
        uint32_t length = readUleb128(stream);
        std::string result(length, '\0');
        if (length) {
            readBytes(stream, result.data(), length);
        }
        return result;
    }

    inline void skipOsuString(std::istream& stream) {
        uint8_t indicator = readU8(stream);
        if (indicator == 0x00) {
            return;
        }
        if (indicator != 0x0b) {
            throw std::runtime_error("Invalid osu string indicator: " + std::to_string(indicator));
        }
        stream.seekg(readUleb128(stream), std::ios::cur);
    }

    // .NET BinaryWriter string: ULEB128 length + UTF-8 bytes (no indicator byte)
    inline std::string readDotnetString(std::istream& stream) {
        uint32_t length = readUleb128(stream);
        std::string result(length, '\0');
        if (length) {
            readBytes(stream, result.data(), length);
        }
        return result;
    }

    // Skip timing points (17 bytes each: double+double+bool)
    inline void skipTimingPoints(std::istream& stream) {
        uint32_t count = readU32(stream);
        stream.seekg(static_cast<std::streamoff>(count) * 17, std::ios::cur);
    }

    inline std::vector<models::Collection> parseCollectionDb(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open: " + path);
        }

        readU32(file); // version
        uint32_t collectionCount = readU32(file);

        std::vector<models::Collection> collections;
        collections.reserve(collectionCount);
        for (uint32_t i = 0; i < collectionCount; ++i) {
            models::Collection collection;
            collection.name = readOsuString(file);
            uint32_t hashCount = readU32(file);
            collection.hashes.reserve(hashCount);
            for (uint32_t j = 0; j < hashCount; ++j) {
                collection.hashes.push_back(readOsuString(file));
            }
            collections.push_back(std::move(collection));
        }
        return collections;
    }

    // Only supports version >= 20191106 (same as CollectionManager).
    // Star rating entries: 1 byte type(8=int32) + 4 bytes mods + 1 byte type(13=double) + 8 bytes stars = 14 bytes each.
    inline std::unordered_map<std::string, models::Beatmap> parseOsuDb(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open: " + path);
        }

        uint32_t version = readU32(file);
        if (version < 20191106) {
            throw std::runtime_error("osu!.db version too old (" + std::to_string(version) + "), need >= 20191106");
        }

        readU32(file); // folder_count
        readBool(file); // account_unlocked
        readI64(file); // date_unlocked
        skipOsuString(file); // player name
        uint32_t beatmapCount = readU32(file);

        std::unordered_map<std::string, models::Beatmap> beatmapMap;
        beatmapMap.reserve(beatmapCount);

        for (uint32_t i = 0; i < beatmapCount; ++i) {
            models::Beatmap beatmap;
            beatmap.artist = readOsuString(file); // artist ASCII
            skipOsuString(file); // artist Unicode
            beatmap.title = readOsuString(file); // title ASCII
            skipOsuString(file); // title Unicode
            beatmap.mapper = readOsuString(file);
            beatmap.difficulty = readOsuString(file);
            skipOsuString(file); // audio filename
            beatmap.md5 = readOsuString(file);
            skipOsuString(file); // .osu filename

            readU8(file); // ranked_status
            readU16(file); // circles
            readU16(file); // sliders
            readU16(file); // spinners
            readI64(file); // last_modified

            // All versions >= 20191106 store these as float32
            readF32(file); // AR
            readF32(file); // CS
            readF32(file); // HP
            readF32(file); // OD
            readF64(file); // slider_velocity

            // 4 star-rating maps (one per game mode).
            // Each entry = 1 byte (type=8/int32) + 4 bytes mods + 1 byte (type=0x0c/float32) + 4 bytes stars = 10 bytes.
            for (int mode = 0; mode < 4; ++mode) {
                uint32_t count = readU32(file);
                if (mode == 0 && count > 0) {
                    readU8(file); // type byte (8 = int32)
                    readU32(file); // mods
                    readU8(file); // type byte (0x0c = float32)
                    beatmap.starRating = readF32(file);
                    --count;
                }
                // Skip remaining entries (10 bytes each: 1+4+1+4)
                file.seekg(static_cast<std::streamoff>(count) * 10, std::ios::cur);
            }

            readU32(file); // drain_time
            readU32(file); // total_time
            readU32(file); // preview_offset

            skipTimingPoints(file);

            beatmap.beatmapId    = readI32(file); // beatmap_id (difficulty)
            beatmap.beatmapSetId = readI32(file); // beatmapset_id
            readI32(file); // thread_id
            readU8(file); // grade_std
            readU8(file); // grade_taiko
            readU8(file); // grade_ctb
            readU8(file); // grade_mania
            readU16(file); // local_offset
            readF32(file); // stack_leniency
            readU8(file); // game_mode
            skipOsuString(file); // source
            skipOsuString(file); // tags
            readU16(file); // online_offset
            skipOsuString(file); // title font
            readBool(file); // unplayed
            readI64(file); // last_played
            readBool(file); // is_osz2
            skipOsuString(file); // folder name
            readI64(file); // last_checked
            readBool(file); // ignore_sounds
            readBool(file); // ignore_skin
            readBool(file); // disable_storyboard
            readBool(file); // disable_video
            readBool(file); // visual_override
            readU32(file); // last_modified2
            readU8(file); // scroll_speed

            if (!beatmap.md5.empty()) {
                beatmapMap[beatmap.md5] = std::move(beatmap);
            }
        }
        return beatmapMap;
    }

    // .osdb (CollectionManager by Piotrekol)
    // File layout (version >= 7, which is all current files):
    //   ULEB128+bytes  outer version string (e.g. "o!dm8")
    //   GZip blob {
    //     ULEB128+bytes  inner version string (same)
    //     double         OA date (days since 1899-12-30)
    //     ULEB128+bytes  editor username
    //     int32          collection count
    //     per collection:
    //       ULEB128+bytes  name
    //       int32          onlineId        (v>=7)
    //       int32          beatmap count
    //       per beatmap:
    //         int32        mapId
    //         int32        mapSetId        (v>=2)
    //         ULEB128+bytes artist          (full only)
    //         ULEB128+bytes title           (full only)
    //         ULEB128+bytes diffName        (full only)
    //         ULEB128+bytes md5
    //         ULEB128+bytes userComment     (v>=4)
    //         byte         playMode        (v>=5 full, or v>=8)
    //         double       starRating      (v>=6 full, or v>=8)
    //       int32          hash-only count (v>=3)
    //       per hash: ULEB128+bytes md5
    //     "By Piotrekol" footer
    //   }

    inline std::vector<uint8_t> gzipDecompress(std::istream& source) {
        std::vector<uint8_t> compressedBytes(
            (std::istreambuf_iterator<char>(source)),
             std::istreambuf_iterator<char>());

        std::vector<uint8_t> output;
        output.resize(std::max<size_t>(compressedBytes.size() * 4, 65536u));

        z_stream zstream{};
        if (inflateInit2(&zstream, 15 + 16) != Z_OK) {  // 15+16 = GZip mode
            throw std::runtime_error("zlib inflateInit2 failed");
        }

        zstream.next_in = compressedBytes.data();
        zstream.avail_in = static_cast<uInt>(compressedBytes.size());

        int zlibResult;
        do {
            if (zstream.total_out >= output.size()) {
                output.resize(output.size() * 2);
            }
            zstream.next_out = output.data() + zstream.total_out;
            zstream.avail_out = static_cast<uInt>(output.size() - zstream.total_out);
            zlibResult = inflate(&zstream, Z_NO_FLUSH);
        } while (zlibResult == Z_OK);

        if (zlibResult != Z_STREAM_END) {
            throw std::runtime_error("GZip decompress failed (zlib code " + std::to_string(zlibResult) + ")");
        }

        output.resize(zstream.total_out);
        inflateEnd(&zstream);
        return output;
    }

    inline std::vector<models::Collection> parseOsdb(std::istream& file) {
        static const std::unordered_map<std::string, int> versionMap = {
            {"o!dm",    1}
           ,{"o!dm2",   2}
           ,{"o!dm3",   3}
           ,{"o!dm4",   4}
           ,{"o!dm5",   5}
           ,{"o!dm6",   6}
           ,{"o!dm7",   7}
           ,{"o!dm8",   8}
           ,{"o!dm7min",1007}
           ,{"o!dm8min",1008}
        };


        std::string versionStr = readDotnetString(file);
        if (auto found = versionMap.find(versionStr); found == versionMap.end()) {
            throw std::runtime_error("Unknown osdb version string: \"" + versionStr + "\"");
        }

        int fileVersion = versionMap.at(versionStr);
        bool minimal = (versionStr.size() >= 3 && versionStr.substr(versionStr.size() - 3) == "min");

        std::string decompressedStr;
        std::istream* stream = &file;
        std::istringstream decompressedStream;

        if (fileVersion >= 7) {
            auto decompressedBytes = gzipDecompress(file);
            decompressedStr.assign(reinterpret_cast<char*>(decompressedBytes.data()), decompressedBytes.size());
            decompressedStream = std::istringstream(decompressedStr, std::ios::binary);
            readDotnetString(decompressedStream); // discard inner version string
            stream = &decompressedStream;
        }

        readF64(*stream); // OA date
        readDotnetString(*stream); // editor username
        int32_t collectionCount = readI32(*stream);

        std::vector<models::Collection> collections;
        collections.reserve(collectionCount);

        for (int32_t i = 0; i < collectionCount; ++i) {
            models::Collection collection;
            collection.name = readDotnetString(*stream);
            if (fileVersion >= 7) {
                readI32(*stream); // onlineId
            }

            int32_t beatmapCount = readI32(*stream);
            collection.beatmaps.reserve(beatmapCount);
            collection.hashes.reserve(beatmapCount);

            for (int32_t j = 0; j < beatmapCount; ++j) {
                const auto beatmapStart = stream->tellg();
                models::Beatmap beatmap;
                try {
                    beatmap.beatmapId = readI32(*stream);
                    if (fileVersion >= 2) {
                        beatmap.beatmapSetId = readI32(*stream);
                    }

                    if (!minimal) {
                        beatmap.artist = readDotnetString(*stream);
                        beatmap.title = readDotnetString(*stream);
                        beatmap.difficulty = readDotnetString(*stream);
                    }

                    beatmap.md5 = readDotnetString(*stream);
                    if (fileVersion >= 4) {
                        readDotnetString(*stream); // userComment
                    }
                    if (fileVersion >= 8 || (fileVersion >= 5 && !minimal)) {
                        readU8(*stream); // playMode
                    }
                    if (fileVersion >= 8 || (fileVersion >= 6 && !minimal)) {
                        beatmap.starRating = readF64(*stream);
                    }

                    collection.hashes.push_back(beatmap.md5);
                    collection.beatmaps.push_back(std::move(beatmap));
                } catch (const std::runtime_error& error) {
                    if (std::string(error.what()) != "Unexpected end of file" || beatmapStart == std::streampos(-1)) {
                        throw;
                    }
                    stream->clear();
                    stream->seekg(beatmapStart);
                    break;
                }
            }

            if (fileVersion >= 3) {
                int32_t hashCount = readI32(*stream);
                for (int32_t j = 0; j < hashCount; ++j) {
                    collection.hashes.push_back(readDotnetString(*stream));
                }
            }

            collections.push_back(std::move(collection));
        }
        return collections;
    }

    inline std::vector<models::Collection> parseOsdb(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open: " + path);
        return parseOsdb(f);
    }

    // Resolve osu!.db metadata into collections
    inline void resolveBeatmaps(std::vector<models::Collection>& collections, const std::unordered_map<std::string, models::Beatmap>& beatmapMap) {
        for (auto& collection : collections) {
            collection.beatmaps.clear();
            for (const auto& hash : collection.hashes) {
                if (auto found = beatmapMap.find(hash); found != beatmapMap.end()) {
                    collection.beatmaps.push_back(found->second);
                } else {
                    models::Beatmap missing;
                    missing.md5 = hash;
                    collection.beatmaps.push_back(std::move(missing));
                }
            }
        }
    }
}

// Write support
namespace stable {

    template<typename T>
    inline void writeVal(std::ostream& s, T v) {
        s.write(reinterpret_cast<const char*>(&v), sizeof(T));
    }

    inline void writeUleb128(std::ostream& s, uint32_t v) {
        do {
            uint8_t byte = v & 0x7F;
            v >>= 7;
            if (v) byte |= 0x80;
            s.put(byte);
        } while (v);
    }

    inline void writeOsuString(std::ostream& s, const std::string& str) {
        if (str.empty()) { s.put(0x00); return; }
        s.put(0x0b);
        writeUleb128(s, static_cast<uint32_t>(str.size()));
        s.write(str.data(), str.size());
    }

    inline void writeDotnetString(std::ostream& s, const std::string& str) {
        writeUleb128(s, static_cast<uint32_t>(str.size()));
        s.write(str.data(), str.size());
    }

    inline std::vector<uint8_t> gzipCompress(const std::vector<uint8_t>& data) {
        z_stream zs{};
        deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
        std::vector<uint8_t> out(data.size() * 2 + 1024);
        zs.next_in   = const_cast<Bytef*>(data.data());
        zs.avail_in  = static_cast<uInt>(data.size());
        zs.next_out  = out.data();
        zs.avail_out = static_cast<uInt>(out.size());
        int res = deflate(&zs, Z_FINISH);
        deflateEnd(&zs);
        if (res != Z_STREAM_END) throw std::runtime_error("gzip compress failed");
        out.resize(zs.total_out);
        return out;
    }

    // Write collection.db (stable native format)
    inline void writeCollectionDb(const std::string& path, const std::vector<models::Collection>& collections) {
        std::ofstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot write: " + path);
        writeVal<uint32_t>(file, 20230803);
        writeVal<uint32_t>(file, static_cast<uint32_t>(collections.size()));
        for (const auto& col : collections) {
            writeOsuString(file, col.name);
            writeVal<uint32_t>(file, static_cast<uint32_t>(col.hashes.size()));
            for (const auto& hash : col.hashes) writeOsuString(file, hash);
        }
    }

    // Write osdb (CollectionManager format o!dm8, compressed)
    inline void writeOsdb(std::ostream& output, const std::vector<models::Collection>& collections) {
        std::ostringstream inner;
        writeDotnetString(inner, "o!dm8");

        auto now = std::chrono::system_clock::now();
        double oaDate = 25569.0 + std::chrono::duration<double>(now.time_since_epoch()).count() / 86400.0;
        writeVal<double>(inner, oaDate);
        writeDotnetString(inner, "");
        writeVal<int32_t>(inner, static_cast<int32_t>(collections.size()));

        for (const auto& col : collections) {
            writeDotnetString(inner, col.name);
            writeVal<int32_t>(inner, 0);
            writeVal<int32_t>(inner, static_cast<int32_t>(col.beatmaps.size()));
            for (const auto& b : col.beatmaps) {
                writeVal<int32_t>(inner, b.beatmapId);
                writeVal<int32_t>(inner, b.beatmapSetId);
                writeDotnetString(inner, b.artist);
                writeDotnetString(inner, b.title);
                writeDotnetString(inner, b.difficulty);
                writeDotnetString(inner, b.md5);
                writeDotnetString(inner, "");  // userComment
                writeVal<uint8_t>(inner, 0);   // playMode
                writeVal<double>(inner, b.starRating);
            }
            writeVal<int32_t>(inner, 0); // hash-only count
        }
        writeDotnetString(inner, "By Piotrekol");

        std::string innerStr = inner.str();
        std::vector<uint8_t> bytes(innerStr.begin(), innerStr.end());
        auto compressed = gzipCompress(bytes);

        writeDotnetString(output, "o!dm8");
        output.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    }

    inline void writeOsdb(const std::string& path, const std::vector<models::Collection>& collections) {
        std::ofstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot write: " + path);
        writeOsdb(file, collections);
    }
}

