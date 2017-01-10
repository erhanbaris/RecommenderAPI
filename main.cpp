#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <iostream>

#pragma warning(disable:4503)

#include "config.h"
#include "core/data/BaseDataSource.h"
#include "core/data/CvsDataSource.h"
//#include "MongoDbDataSource.h"
#include "core/algoritm/SlopeOne.h"
#include <vector>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <functional>
#include <locale>
#include <sstream>

#include "crow.h"

#include "core/algoritm/PearsonCorrelationCoefficien.h"
#include "model/Rating.h"

#include "core/types.h"
#include "core/data/BaseDataInfo.h"


#include <map>


#pragma execution_character_set("utf-8")

using namespace std;
using namespace std::chrono;
using namespace model;
using namespace core::algoritm;


core::data::BaseDataSource<core::data::DataInfo> *dataSource;
PearsonCorrelationCoefficien mDistance;

std::string escapeJsonString(const std::string &input) {
    std::ostringstream ss;
    for (auto iter = input.cbegin(); iter != input.cend(); iter++) {
        //C++98/03:
        //for (std::string::const_iterator iter = input.begin(); iter != input.end(); iter++) {
        switch (*iter) {
            case '\\':
                ss << "\\\\";
                break;
            case '"':
                ss << "\\\"";
                break;
            case '/':
                ss << "\\/";
                break;
            case '\b':
                ss << "\\b";
                break;
            case '\f':
                ss << "\\f";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '\r':
                ss << "\\r";
                break;
            case '\t':
                ss << "\\t";
                break;
            default:
                ss << *iter;
                break;
        }
    }
    return ss.str();
}

vector<pair<PRODUCT_TYPE, wstring>> recommend(USER_TYPE userId) {
    vector<pair<PRODUCT_TYPE, wstring>> recommendedProducts;
    CUSTOM_MAP<PRODUCT_TYPE, float> recommendedProductIds;
    INT_INIT_MAP(recommendedProductIds);

    CUSTOM_MAP<PRODUCT_TYPE, RATE_TYPE> userRatings;
    INT_INIT_MAP(userRatings);

    auto &userDatas = dataSource->Data()->userMap[userId].Products;
    auto endData = userDatas.end();
    for (auto it = userDatas.begin(); it != endData; ++it)
        userRatings[(*it)->ProductId] = (*it)->Rating;

    const auto &results = dataSource->Data()->slopeOne.Predict(userRatings);
    //const auto& results = mDistance.Recommend(&dataSource->Data()->userRatings, userId);
    if (results.size() > 0) {
        auto nearestUser = results.at(0);

        auto nearestUserRatings = dataSource->Data()->userRatings[nearestUser.first];
        auto currentUserRatings = dataSource->Data()->userRatings[userId];
        auto currentUserRatingsEnd = currentUserRatings.end();

        float totalDistance = 0.0;
        size_t counter = 0;
        auto resultsEnd = results.cend();
        for (auto it = results.cbegin(); it != resultsEnd && counter < KNEAR_SIZE; ++it) {
            totalDistance += it->second;
            ++counter;
        }

        counter = 0;
        for (auto nearest = results.cbegin(); nearest != resultsEnd && counter < KNEAR_SIZE; ++nearest) {
            float weight = nearest->second / totalDistance;
            auto neighborRatings = dataSource->Data()->userRatings[nearest->first];

            auto neighborRatingsEnd = neighborRatings.cend();
            for (auto artist = neighborRatings.cbegin(); artist != neighborRatingsEnd; ++artist) {
                const auto &artistItem = (*artist);

                bool hasProduct = false;
                for (auto it = currentUserRatings.begin(); it != currentUserRatingsEnd; ++it) {
                    if (it->ProductId == artistItem.ProductId) {
                        hasProduct = true;
                        break;
                    }
                }

                if (hasProduct) {
                    auto recommendItem = recommendedProductIds.find(artistItem.ProductId);
                    if (recommendItem == recommendedProductIds.end())
                        recommendedProductIds[artistItem.ProductId] = static_cast<float>(artistItem.Rate) * weight;
                    else
                        recommendItem->second = recommendItem->second + (static_cast<float>(artistItem.Rate) * weight);
                }
            }

            ++counter;
        }
    }


    vector<pair<PRODUCT_TYPE, float>> sortIds;
    auto recommendedProductIdsEnd = recommendedProductIds.end();
    for (auto it = recommendedProductIds.begin(); it != recommendedProductIdsEnd; ++it)
        sortIds.push_back(pair<PRODUCT_TYPE, float>(it->first, it->second));

    std::sort(sortIds.begin(), sortIds.end(),
              [](pair<PRODUCT_TYPE, float> const &left, pair<PRODUCT_TYPE, float> const &right) {
                  return left.second < right.second;
              });

    for (auto id = sortIds.begin(); id != sortIds.end(); ++id)
        recommendedProducts.push_back(
                pair<PRODUCT_TYPE, wstring>(id->first, dataSource->Data()->productInfos[id->first]));

    return recommendedProducts;
}

typedef char BYTE;

struct StaticFileParser {
    static char *StaticFolder;
    struct context {
    };


    StaticFileParser() {
        INIT_MAP(mimeTypes, "", "-");
        mimeTypes["3dm"] = "x-world/x-3dmf";
        mimeTypes["3dmf"] = "x-world/x-3dmf";
        mimeTypes["a"] = "application/octet-stream";
        mimeTypes["aab"] = "application/x-authorware-bin";
        mimeTypes["aam"] = "application/x-authorware-map";
        mimeTypes["aas"] = "application/x-authorware-seg";
        mimeTypes["abc"] = "text/vnd.abc";
        mimeTypes["acgi"] = "text/html";
        mimeTypes["afl"] = "video/animaflex";
        mimeTypes["ai"] = "application/postscript";
        mimeTypes["aif"] = "audio/aiff";
        mimeTypes["aif"] = "audio/x-aiff";
        mimeTypes["aifc"] = "audio/aiff";
        mimeTypes["aifc"] = "audio/x-aiff";
        mimeTypes["aiff"] = "audio/aiff";
        mimeTypes["aiff"] = "audio/x-aiff";
        mimeTypes["aim"] = "application/x-aim";
        mimeTypes["aip"] = "text/x-audiosoft-intra";
        mimeTypes["ani"] = "application/x-navi-animation";
        mimeTypes["aos"] = "application/x-nokia-9000-communicator-add-on-software";
        mimeTypes["aps"] = "application/mime";
        mimeTypes["arc"] = "application/octet-stream";
        mimeTypes["arj"] = "application/arj";
        mimeTypes["arj"] = "application/octet-stream";
        mimeTypes["art"] = "image/x-jg";
        mimeTypes["asf"] = "video/x-ms-asf";
        mimeTypes["asm"] = "text/x-asm";
        mimeTypes["asp"] = "text/asp";
        mimeTypes["asx"] = "application/x-mplayer2";
        mimeTypes["asx"] = "video/x-ms-asf";
        mimeTypes["asx"] = "video/x-ms-asf-plugin";
        mimeTypes["au"] = "audio/basic";
        mimeTypes["au"] = "audio/x-au";
        mimeTypes["avi"] = "application/x-troff-msvideo";
        mimeTypes["avi"] = "video/avi";
        mimeTypes["avi"] = "video/msvideo";
        mimeTypes["avi"] = "video/x-msvideo";
        mimeTypes["avs"] = "video/avs-video";
        mimeTypes["bcpio"] = "application/x-bcpio";
        mimeTypes["bin"] = "application/mac-binary";
        mimeTypes["bin"] = "application/macbinary";
        mimeTypes["bin"] = "application/octet-stream";
        mimeTypes["bin"] = "application/x-binary";
        mimeTypes["bin"] = "application/x-macbinary";
        mimeTypes["bm"] = "image/bmp";
        mimeTypes["bmp"] = "image/bmp";
        mimeTypes["bmp"] = "image/x-windows-bmp";
        mimeTypes["boo"] = "application/book";
        mimeTypes["book"] = "application/book";
        mimeTypes["boz"] = "application/x-bzip2";
        mimeTypes["bsh"] = "application/x-bsh";
        mimeTypes["bz"] = "application/x-bzip";
        mimeTypes["bz2"] = "application/x-bzip2";
        mimeTypes["c"] = "text/plain";
        mimeTypes["c"] = "text/x-c";
        mimeTypes["c++"] = "text/plain";
        mimeTypes["cat"] = "application/vnd.ms-pki.seccat";
        mimeTypes["cc"] = "text/plain";
        mimeTypes["cc"] = "text/x-c";
        mimeTypes["ccad"] = "application/clariscad";
        mimeTypes["cco"] = "application/x-cocoa";
        mimeTypes["cdf"] = "application/cdf";
        mimeTypes["cdf"] = "application/x-cdf";
        mimeTypes["cdf"] = "application/x-netcdf";
        mimeTypes["cer"] = "application/pkix-cert";
        mimeTypes["cer"] = "application/x-x509-ca-cert";
        mimeTypes["cha"] = "application/x-chat";
        mimeTypes["chat"] = "application/x-chat";
        mimeTypes["class"] = "application/java";
        mimeTypes["class"] = "application/java-byte-code";
        mimeTypes["class"] = "application/x-java-class";
        mimeTypes["com"] = "application/octet-stream";
        mimeTypes["com"] = "text/plain";
        mimeTypes["conf"] = "text/plain";
        mimeTypes["cpio"] = "application/x-cpio";
        mimeTypes["cpp"] = "text/x-c";
        mimeTypes["cpt"] = "application/mac-compactpro";
        mimeTypes["cpt"] = "application/x-compactpro";
        mimeTypes["cpt"] = "application/x-cpt";
        mimeTypes["crl"] = "application/pkcs-crl";
        mimeTypes["crl"] = "application/pkix-crl";
        mimeTypes["crt"] = "application/pkix-cert";
        mimeTypes["crt"] = "application/x-x509-ca-cert";
        mimeTypes["crt"] = "application/x-x509-user-cert";
        mimeTypes["csh"] = "application/x-csh";
        mimeTypes["csh"] = "text/x-script.csh";
        mimeTypes["css"] = "application/x-pointplus";
        mimeTypes["css"] = "text/css";
        mimeTypes["cxx"] = "text/plain";
        mimeTypes["dcr"] = "application/x-director";
        mimeTypes["deepv"] = "application/x-deepv";
        mimeTypes["def"] = "text/plain";
        mimeTypes["der"] = "application/x-x509-ca-cert";
        mimeTypes["dif"] = "video/x-dv";
        mimeTypes["dir"] = "application/x-director";
        mimeTypes["dl"] = "video/dl";
        mimeTypes["dl"] = "video/x-dl";
        mimeTypes["doc"] = "application/msword";
        mimeTypes["dot"] = "application/msword";
        mimeTypes["dp"] = "application/commonground";
        mimeTypes["drw"] = "application/drafting";
        mimeTypes["dump"] = "application/octet-stream";
        mimeTypes["dv"] = "video/x-dv";
        mimeTypes["dvi"] = "application/x-dvi";
        mimeTypes["dwf"] = "drawing/x-dwf";
        mimeTypes["dwf"] = "model/vnd.dwf";
        mimeTypes["dwg"] = "application/acad";
        mimeTypes["dwg"] = "image/vnd.dwg";
        mimeTypes["dwg"] = "image/x-dwg";
        mimeTypes["dxf"] = "application/dxf";
        mimeTypes["dxf"] = "image/vnd.dwg";
        mimeTypes["dxf"] = "image/x-dwg";
        mimeTypes["dxr"] = "application/x-director";
        mimeTypes["el"] = "text/x-script.elisp";
        mimeTypes["elc"] = "application/x-bytecode.elisp";
        mimeTypes["elc"] = "application/x-elc";
        mimeTypes["env"] = "application/x-envoy";
        mimeTypes["eps"] = "application/postscript";
        mimeTypes["es"] = "application/x-esrehber";
        mimeTypes["etx"] = "text/x-setext";
        mimeTypes["evy"] = "application/envoy";
        mimeTypes["evy"] = "application/x-envoy";
        mimeTypes["exe"] = "application/octet-stream";
        mimeTypes["f"] = "text/plain";
        mimeTypes["f"] = "text/x-fortran";
        mimeTypes["f77"] = "text/x-fortran";
        mimeTypes["f90"] = "text/plain";
        mimeTypes["f90"] = "text/x-fortran";
        mimeTypes["fdf"] = "application/vnd.fdf";
        mimeTypes["fif"] = "application/fractals";
        mimeTypes["fif"] = "image/fif";
        mimeTypes["fli"] = "video/fli";
        mimeTypes["fli"] = "video/x-fli";
        mimeTypes["flo"] = "image/florian";
        mimeTypes["flx"] = "text/vnd.fmi.flexstor";
        mimeTypes["fmf"] = "video/x-atomic3d-feature";
        mimeTypes["for"] = "text/plain";
        mimeTypes["for"] = "text/x-fortran";
        mimeTypes["fpx"] = "image/vnd.fpx";
        mimeTypes["fpx"] = "image/vnd.net-fpx";
        mimeTypes["frl"] = "application/freeloader";
        mimeTypes["funk"] = "audio/make";
        mimeTypes["g"] = "text/plain";
        mimeTypes["g3"] = "image/g3fax";
        mimeTypes["gif"] = "image/gif";
        mimeTypes["gl"] = "video/gl";
        mimeTypes["gl"] = "video/x-gl";
        mimeTypes["gsd"] = "audio/x-gsm";
        mimeTypes["gsm"] = "audio/x-gsm";
        mimeTypes["gsp"] = "application/x-gsp";
        mimeTypes["gss"] = "application/x-gss";
        mimeTypes["gtar"] = "application/x-gtar";
        mimeTypes["gz"] = "application/x-compressed";
        mimeTypes["gz"] = "application/x-gzip";
        mimeTypes["gzip"] = "application/x-gzip";
        mimeTypes["gzip"] = "multipart/x-gzip";
        mimeTypes["h"] = "text/plain";
        mimeTypes["h"] = "text/x-h";
        mimeTypes["hdf"] = "application/x-hdf";
        mimeTypes["help"] = "application/x-helpfile";
        mimeTypes["hgl"] = "application/vnd.hp-hpgl";
        mimeTypes["hh"] = "text/plain";
        mimeTypes["hh"] = "text/x-h";
        mimeTypes["hlb"] = "text/x-script";
        mimeTypes["hlp"] = "application/hlp";
        mimeTypes["hlp"] = "application/x-helpfile";
        mimeTypes["hlp"] = "application/x-winhelp";
        mimeTypes["hpg"] = "application/vnd.hp-hpgl";
        mimeTypes["hpgl"] = "application/vnd.hp-hpgl";
        mimeTypes["hqx"] = "application/binhex";
        mimeTypes["hqx"] = "application/binhex4";
        mimeTypes["hqx"] = "application/mac-binhex";
        mimeTypes["hqx"] = "application/mac-binhex40";
        mimeTypes["hqx"] = "application/x-binhex40";
        mimeTypes["hqx"] = "application/x-mac-binhex40";
        mimeTypes["hta"] = "application/hta";
        mimeTypes["htc"] = "text/x-component";
        mimeTypes["htm"] = "text/html";
        mimeTypes["html"] = "text/html";
        mimeTypes["htmls"] = "text/html";
        mimeTypes["htt"] = "text/webviewhtml";
        mimeTypes["htx"] = "text/html";
        mimeTypes["ice"] = "x-conference/x-cooltalk";
        mimeTypes["ico"] = "image/x-icon";
        mimeTypes["idc"] = "text/plain";
        mimeTypes["ief"] = "image/ief";
        mimeTypes["iefs"] = "image/ief";
        mimeTypes["iges"] = "application/iges";
        mimeTypes["iges"] = "model/iges";
        mimeTypes["igs"] = "application/iges";
        mimeTypes["igs"] = "model/iges";
        mimeTypes["ima"] = "application/x-ima";
        mimeTypes["imap"] = "application/x-httpd-imap";
        mimeTypes["inf"] = "application/inf";
        mimeTypes["ins"] = "application/x-internett-signup";
        mimeTypes["ip"] = "application/x-ip2";
        mimeTypes["isu"] = "video/x-isvideo";
        mimeTypes["it"] = "audio/it";
        mimeTypes["iv"] = "application/x-inventor";
        mimeTypes["ivr"] = "i-world/i-vrml";
        mimeTypes["ivy"] = "application/x-livescreen";
        mimeTypes["jam"] = "audio/x-jam";
        mimeTypes["jav"] = "text/plain";
        mimeTypes["jav"] = "text/x-java-source";
        mimeTypes["java"] = "text/plain";
        mimeTypes["java"] = "text/x-java-source";
        mimeTypes["jcm"] = "application/x-java-commerce";
        mimeTypes["jfif"] = "image/jpeg";
        mimeTypes["jfif"] = "image/pjpeg";
        mimeTypes["jfif-tbnl"] = "image/jpeg";
        mimeTypes["jpe"] = "image/jpeg";
        mimeTypes["jpe"] = "image/pjpeg";
        mimeTypes["jpeg"] = "image/jpeg";
        mimeTypes["jpeg"] = "image/pjpeg";
        mimeTypes["jpg"] = "image/jpeg";
        mimeTypes["jpg"] = "image/pjpeg";
        mimeTypes["jps"] = "image/x-jps";
        mimeTypes["js"] = "application/x-javascript";
        mimeTypes["js"] = "application/javascript";
        mimeTypes["js"] = "application/ecmascript";
        mimeTypes["js"] = "text/javascript";
        mimeTypes["js"] = "text/ecmascript";
        mimeTypes["json"] = "application/javascript";
        mimeTypes["jut"] = "image/jutvision";
        mimeTypes["kar"] = "audio/midi";
        mimeTypes["kar"] = "music/x-karaoke";
        mimeTypes["ksh"] = "application/x-ksh";
        mimeTypes["ksh"] = "text/x-script.ksh";
        mimeTypes["la"] = "audio/nspaudio";
        mimeTypes["la"] = "audio/x-nspaudio";
        mimeTypes["lam"] = "audio/x-liveaudio";
        mimeTypes["latex"] = "application/x-latex";
        mimeTypes["lha"] = "application/lha";
        mimeTypes["lha"] = "application/octet-stream";
        mimeTypes["lha"] = "application/x-lha";
        mimeTypes["lhx"] = "application/octet-stream";
        mimeTypes["list"] = "text/plain";
        mimeTypes["lma"] = "audio/nspaudio";
        mimeTypes["lma"] = "audio/x-nspaudio";
        mimeTypes["log"] = "text/plain";
        mimeTypes["lsp"] = "application/x-lisp";
        mimeTypes["lsp"] = "text/x-script.lisp";
        mimeTypes["lst"] = "text/plain";
        mimeTypes["lsx"] = "text/x-la-asf";
        mimeTypes["ltx"] = "application/x-latex";
        mimeTypes["lzh"] = "application/octet-stream";
        mimeTypes["lzh"] = "application/x-lzh";
        mimeTypes["lzx"] = "application/lzx";
        mimeTypes["lzx"] = "application/octet-stream";
        mimeTypes["lzx"] = "application/x-lzx";
        mimeTypes["m"] = "text/plain";
        mimeTypes["m"] = "text/x-m";
        mimeTypes["m1v"] = "video/mpeg";
        mimeTypes["m2a"] = "audio/mpeg";
        mimeTypes["m2v"] = "video/mpeg";
        mimeTypes["m3u"] = "audio/x-mpequrl";
        mimeTypes["man"] = "application/x-troff-man";
        mimeTypes["map"] = "application/x-navimap";
        mimeTypes["mar"] = "text/plain";
        mimeTypes["mbd"] = "application/mbedlet";
        mimeTypes["mc$"] = "application/x-magic-cap-package-1.0";
        mimeTypes["mcd"] = "application/mcad";
        mimeTypes["mcd"] = "application/x-mathcad";
        mimeTypes["mcf"] = "image/vasa";
        mimeTypes["mcf"] = "text/mcf";
        mimeTypes["mcp"] = "application/netmc";
        mimeTypes["me"] = "application/x-troff-me";
        mimeTypes["mht"] = "message/rfc822";
        mimeTypes["mhtml"] = "message/rfc822";
        mimeTypes["mid"] = "application/x-midi";
        mimeTypes["mid"] = "audio/midi";
        mimeTypes["mid"] = "audio/x-mid";
        mimeTypes["mid"] = "audio/x-midi";
        mimeTypes["mid"] = "music/crescendo";
        mimeTypes["mid"] = "x-music/x-midi";
        mimeTypes["midi"] = "application/x-midi";
        mimeTypes["midi"] = "audio/midi";
        mimeTypes["midi"] = "audio/x-mid";
        mimeTypes["midi"] = "audio/x-midi";
        mimeTypes["midi"] = "music/crescendo";
        mimeTypes["midi"] = "x-music/x-midi";
        mimeTypes["mif"] = "application/x-frame";
        mimeTypes["mif"] = "application/x-mif";
        mimeTypes["mime"] = "message/rfc822";
        mimeTypes["mime"] = "www/mime";
        mimeTypes["mjf"] = "audio/x-vnd.audioexplosion.mjuicemediafile";
        mimeTypes["mjpg"] = "video/x-motion-jpeg";
        mimeTypes["mm"] = "application/base64";
        mimeTypes["mm"] = "application/x-meme";
        mimeTypes["mme"] = "application/base64";
        mimeTypes["mod"] = "audio/mod";
        mimeTypes["mod"] = "audio/x-mod";
        mimeTypes["moov"] = "video/quicktime";
        mimeTypes["mov"] = "video/quicktime";
        mimeTypes["movie"] = "video/x-sgi-movie";
        mimeTypes["mp2"] = "audio/mpeg";
        mimeTypes["mp2"] = "audio/x-mpeg";
        mimeTypes["mp2"] = "video/mpeg";
        mimeTypes["mp2"] = "video/x-mpeg";
        mimeTypes["mp2"] = "video/x-mpeq2a";
        mimeTypes["mp3"] = "audio/mpeg3";
        mimeTypes["mp3"] = "audio/x-mpeg-3";
        mimeTypes["mp3"] = "video/mpeg";
        mimeTypes["mp3"] = "video/x-mpeg";
        mimeTypes["mpa"] = "audio/mpeg";
        mimeTypes["mpa"] = "video/mpeg";
        mimeTypes["mpc"] = "application/x-project";
        mimeTypes["mpe"] = "video/mpeg";
        mimeTypes["mpeg"] = "video/mpeg";
        mimeTypes["mpg"] = "audio/mpeg";
        mimeTypes["mpg"] = "video/mpeg";
        mimeTypes["mpga"] = "audio/mpeg";
        mimeTypes["mpp"] = "application/vnd.ms-project";
        mimeTypes["mpt"] = "application/x-project";
        mimeTypes["mpv"] = "application/x-project";
        mimeTypes["mpx"] = "application/x-project";
        mimeTypes["mrc"] = "application/marc";
        mimeTypes["ms"] = "application/x-troff-ms";
        mimeTypes["mv"] = "video/x-sgi-movie";
        mimeTypes["my"] = "audio/make";
        mimeTypes["mzz"] = "application/x-vnd.audioexplosion.mzz";
        mimeTypes["nap"] = "image/naplps";
        mimeTypes["naplps"] = "image/naplps";
        mimeTypes["nc"] = "application/x-netcdf";
        mimeTypes["ncm"] = "application/vnd.nokia.configuration-message";
        mimeTypes["nif"] = "image/x-niff";
        mimeTypes["niff"] = "image/x-niff";
        mimeTypes["nix"] = "application/x-mix-transfer";
        mimeTypes["nsc"] = "application/x-conference";
        mimeTypes["nvd"] = "application/x-navidoc";
        mimeTypes["o"] = "application/octet-stream";
        mimeTypes["oda"] = "application/oda";
        mimeTypes["omc"] = "application/x-omc";
        mimeTypes["omcd"] = "application/x-omcdatamaker";
        mimeTypes["omcr"] = "application/x-omcregerator";
        mimeTypes["p"] = "text/x-pascal";
        mimeTypes["p10"] = "application/pkcs10";
        mimeTypes["p10"] = "application/x-pkcs10";
        mimeTypes["p12"] = "application/pkcs-12";
        mimeTypes["p12"] = "application/x-pkcs12";
        mimeTypes["p7a"] = "application/x-pkcs7-signature";
        mimeTypes["p7c"] = "application/pkcs7-mime";
        mimeTypes["p7c"] = "application/x-pkcs7-mime";
        mimeTypes["p7m"] = "application/pkcs7-mime";
        mimeTypes["p7m"] = "application/x-pkcs7-mime";
        mimeTypes["p7r"] = "application/x-pkcs7-certreqresp";
        mimeTypes["p7s"] = "application/pkcs7-signature";
        mimeTypes["part"] = "application/pro";
        mimeTypes["pas"] = "text/pascal";
        mimeTypes["pbm"] = "image/x-portable-bitmap";
        mimeTypes["pcl"] = "application/vnd.hp-pcl";
        mimeTypes["pcl"] = "application/x-pcl";
        mimeTypes["pct"] = "image/x-pict";
        mimeTypes["pcx"] = "image/x-pcx";
        mimeTypes["pdb"] = "chemical/x-pdb";
        mimeTypes["pdf"] = "application/pdf";
        mimeTypes["pfunk"] = "audio/make";
        mimeTypes["pfunk"] = "audio/make.my.funk";
        mimeTypes["pgm"] = "image/x-portable-graymap";
        mimeTypes["pgm"] = "image/x-portable-greymap";
        mimeTypes["pic"] = "image/pict";
        mimeTypes["pict"] = "image/pict";
        mimeTypes["pkg"] = "application/x-newton-compatible-pkg";
        mimeTypes["pko"] = "application/vnd.ms-pki.pko";
        mimeTypes["pl"] = "text/plain";
        mimeTypes["pl"] = "text/x-script.perl";
        mimeTypes["plx"] = "application/x-pixclscript";
        mimeTypes["pm"] = "image/x-xpixmap";
        mimeTypes["pm"] = "text/x-script.perl-module";
        mimeTypes["pm4"] = "application/x-pagemaker";
        mimeTypes["pm5"] = "application/x-pagemaker";
        mimeTypes["png"] = "image/png";
        mimeTypes["pnm"] = "application/x-portable-anymap";
        mimeTypes["pnm"] = "image/x-portable-anymap";
        mimeTypes["pot"] = "application/mspowerpoint";
        mimeTypes["pot"] = "application/vnd.ms-powerpoint";
        mimeTypes["pov"] = "model/x-pov";
        mimeTypes["ppa"] = "application/vnd.ms-powerpoint";
        mimeTypes["ppm"] = "image/x-portable-pixmap";
        mimeTypes["pps"] = "application/mspowerpoint";
        mimeTypes["pps"] = "application/vnd.ms-powerpoint";
        mimeTypes["ppt"] = "application/mspowerpoint";
        mimeTypes["ppt"] = "application/powerpoint";
        mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
        mimeTypes["ppt"] = "application/x-mspowerpoint";
        mimeTypes["ppz"] = "application/mspowerpoint";
        mimeTypes["pre"] = "application/x-freelance";

        mimeTypes["ps"] = "application/postscript";
        mimeTypes["psd"] = "application/octet-stream";
        mimeTypes["pvu"] = "paleovu/x-pv";
        mimeTypes["pwz"] = "application/vnd.ms-powerpoint";
        mimeTypes["py"] = "text/x-script.phyton";
        mimeTypes["pyc"] = "application/x-bytecode.python";
        mimeTypes["qcp"] = "audio/vnd.qcelp";
        mimeTypes["qd3"] = "x-world/x-3dmf";
        mimeTypes["qd3d"] = "x-world/x-3dmf";
        mimeTypes["qif"] = "image/x-quicktime";
        mimeTypes["qt"] = "video/quicktime";
        mimeTypes["qtc"] = "video/x-qtc";
        mimeTypes["qti"] = "image/x-quicktime";
        mimeTypes["qtif"] = "image/x-quicktime";
        mimeTypes["ra"] = "audio/x-pn-realaudio";
        mimeTypes["ra"] = "audio/x-pn-realaudio-plugin";
        mimeTypes["ra"] = "audio/x-realaudio";
        mimeTypes["ram"] = "audio/x-pn-realaudio";
        mimeTypes["ras"] = "application/x-cmu-raster";
        mimeTypes["ras"] = "image/cmu-raster";
        mimeTypes["ras"] = "image/x-cmu-raster";
        mimeTypes["rast"] = "image/cmu-raster";
        mimeTypes["rexx"] = "text/x-script.rexx";
        mimeTypes["rf"] = "image/vnd.rn-realflash";
        mimeTypes["rgb"] = "image/x-rgb";
        mimeTypes["rm"] = "application/vnd.rn-realmedia";
        mimeTypes["rm"] = "audio/x-pn-realaudio";
        mimeTypes["rmi"] = "audio/mid";
        mimeTypes["rmm"] = "audio/x-pn-realaudio";
        mimeTypes["rmp"] = "audio/x-pn-realaudio";
        mimeTypes["rmp"] = "audio/x-pn-realaudio-plugin";
        mimeTypes["rng"] = "application/ringing-tones";
        mimeTypes["rng"] = "application/vnd.nokia.ringing-tone";
        mimeTypes["rnx"] = "application/vnd.rn-realplayer";
        mimeTypes["roff"] = "application/x-troff";
        mimeTypes["rp"] = "image/vnd.rn-realpix";
        mimeTypes["rpm"] = "audio/x-pn-realaudio-plugin";
        mimeTypes["rt"] = "text/richtext";
        mimeTypes["rt"] = "text/vnd.rn-realtext";
        mimeTypes["rtf"] = "application/rtf";
        mimeTypes["rtf"] = "application/x-rtf";
        mimeTypes["rtf"] = "text/richtext";
        mimeTypes["rtx"] = "application/rtf";
        mimeTypes["rtx"] = "text/richtext";
        mimeTypes["rv"] = "video/vnd.rn-realvideo";
        mimeTypes["s"] = "text/x-asm";
        mimeTypes["s3m"] = "audio/s3m";
        mimeTypes["saveme"] = "application/octet-stream";
        mimeTypes["sbk"] = "application/x-tbook";
        mimeTypes["scm"] = "application/x-lotusscreencam";
        mimeTypes["scm"] = "text/x-script.guile";
        mimeTypes["scm"] = "text/x-script.scheme";
        mimeTypes["scm"] = "video/x-scm";
        mimeTypes["sdml"] = "text/plain";
        mimeTypes["sdp"] = "application/sdp";
        mimeTypes["sdp"] = "application/x-sdp";
        mimeTypes["sdr"] = "application/sounder";
        mimeTypes["sea"] = "application/sea";
        mimeTypes["sea"] = "application/x-sea";
        mimeTypes["set"] = "application/set";
        mimeTypes["sgm"] = "text/sgml";
        mimeTypes["sgm"] = "text/x-sgml";
        mimeTypes["sgml"] = "text/sgml";
        mimeTypes["sgml"] = "text/x-sgml";
        mimeTypes["sh"] = "application/x-bsh";
        mimeTypes["sh"] = "application/x-sh";
        mimeTypes["sh"] = "application/x-shar";
        mimeTypes["sh"] = "text/x-script.sh";
        mimeTypes["shar"] = "application/x-bsh";
        mimeTypes["shar"] = "application/x-shar";
        mimeTypes["shtml"] = "text/html";
        mimeTypes["shtml"] = "text/x-server-parsed-html";
        mimeTypes["sid"] = "audio/x-psid";
        mimeTypes["sit"] = "application/x-sit";
        mimeTypes["sit"] = "application/x-stuffit";
        mimeTypes["skd"] = "application/x-koan";
        mimeTypes["skm"] = "application/x-koan";
        mimeTypes["skp"] = "application/x-koan";
        mimeTypes["skt"] = "application/x-koan";
        mimeTypes["sl"] = "application/x-seelogo";
        mimeTypes["smi"] = "application/smil";
        mimeTypes["smil"] = "application/smil";
        mimeTypes["snd"] = "audio/basic";
        mimeTypes["snd"] = "audio/x-adpcm";
        mimeTypes["sol"] = "application/solids";
        mimeTypes["spc"] = "application/x-pkcs7-certificates";
        mimeTypes["spc"] = "text/x-speech";
        mimeTypes["spl"] = "application/futuresplash";
        mimeTypes["spr"] = "application/x-sprite";
        mimeTypes["sprite"] = "application/x-sprite";
        mimeTypes["src"] = "application/x-wais-source";
        mimeTypes["ssi"] = "text/x-server-parsed-html";
        mimeTypes["ssm"] = "application/streamingmedia";
        mimeTypes["sst"] = "application/vnd.ms-pki.certstore";
        mimeTypes["step"] = "application/step";
        mimeTypes["stl"] = "application/sla";
        mimeTypes["stl"] = "application/vnd.ms-pki.stl";
        mimeTypes["stl"] = "application/x-navistyle";
        mimeTypes["stp"] = "application/step";
        mimeTypes["sv4cpio"] = "application/x-sv4cpio";
        mimeTypes["sv4crc"] = "application/x-sv4crc";
        mimeTypes["svf"] = "image/vnd.dwg";
        mimeTypes["svf"] = "image/x-dwg";
        mimeTypes["svr"] = "application/x-world";
        mimeTypes["svr"] = "x-world/x-svr";
        mimeTypes["swf"] = "application/x-shockwave-flash";
        mimeTypes["t"] = "application/x-troff";
        mimeTypes["talk"] = "text/x-speech";
        mimeTypes["tar"] = "application/x-tar";
        mimeTypes["tbk"] = "application/toolbook";
        mimeTypes["tbk"] = "application/x-tbook";
        mimeTypes["tcl"] = "application/x-tcl";
        mimeTypes["tcl"] = "text/x-script.tcl";
        mimeTypes["tcsh"] = "text/x-script.tcsh";
        mimeTypes["tex"] = "application/x-tex";
        mimeTypes["texi"] = "application/x-texinfo";
        mimeTypes["texinfo"] = "application/x-texinfo";
        mimeTypes["text"] = "application/plain";
        mimeTypes["text"] = "text/plain";
        mimeTypes["tgz"] = "application/gnutar";
        mimeTypes["tgz"] = "application/x-compressed";
        mimeTypes["tif"] = "image/tiff";
        mimeTypes["tif"] = "image/x-tiff";
        mimeTypes["tiff"] = "image/tiff";
        mimeTypes["tiff"] = "image/x-tiff";
        mimeTypes["tr"] = "application/x-troff";
        mimeTypes["tsi"] = "audio/tsp-audio";
        mimeTypes["tsp"] = "application/dsptype";
        mimeTypes["tsp"] = "audio/tsplayer";
        mimeTypes["tsv"] = "text/tab-separated-values";
        mimeTypes["turbot"] = "image/florian";
        mimeTypes["txt"] = "text/plain";
        mimeTypes["uil"] = "text/x-uil";
        mimeTypes["uni"] = "text/uri-list";
        mimeTypes["unis"] = "text/uri-list";
        mimeTypes["unv"] = "application/i-deas";
        mimeTypes["uri"] = "text/uri-list";
        mimeTypes["uris"] = "text/uri-list";
        mimeTypes["ustar"] = "application/x-ustar";
        mimeTypes["ustar"] = "multipart/x-ustar";
        mimeTypes["uu"] = "application/octet-stream";
        mimeTypes["uu"] = "text/x-uuencode";
        mimeTypes["uue"] = "text/x-uuencode";
        mimeTypes["vcd"] = "application/x-cdlink";
        mimeTypes["vcs"] = "text/x-vcalendar";
        mimeTypes["vda"] = "application/vda";
        mimeTypes["vdo"] = "video/vdo";
        mimeTypes["vew"] = "application/groupwise";
        mimeTypes["viv"] = "video/vivo";
        mimeTypes["viv"] = "video/vnd.vivo";
        mimeTypes["vivo"] = "video/vivo";
        mimeTypes["vivo"] = "video/vnd.vivo";
        mimeTypes["vmd"] = "application/vocaltec-media-desc";
        mimeTypes["vmf"] = "application/vocaltec-media-file";
        mimeTypes["voc"] = "audio/voc";
        mimeTypes["voc"] = "audio/x-voc";
        mimeTypes["vos"] = "video/vosaic";
        mimeTypes["vox"] = "audio/voxware";
        mimeTypes["vqe"] = "audio/x-twinvq-plugin";
        mimeTypes["vqf"] = "audio/x-twinvq";
        mimeTypes["vql"] = "audio/x-twinvq-plugin";
        mimeTypes["vrml"] = "application/x-vrml";
        mimeTypes["vrml"] = "model/vrml";
        mimeTypes["vrml"] = "x-world/x-vrml";
        mimeTypes["vrt"] = "x-world/x-vrt";
        mimeTypes["vsd"] = "application/x-visio";
        mimeTypes["vst"] = "application/x-visio";
        mimeTypes["vsw"] = "application/x-visio";
        mimeTypes["w60"] = "application/wordperfect6.0";
        mimeTypes["w61"] = "application/wordperfect6.1";
        mimeTypes["w6w"] = "application/msword";
        mimeTypes["wav"] = "audio/wav";
        mimeTypes["wav"] = "audio/x-wav";
        mimeTypes["wb1"] = "application/x-qpro";
        mimeTypes["wbmp"] = "image/vnd.wap.wbmp";
        mimeTypes["web"] = "application/vnd.xara";
        mimeTypes["wiz"] = "application/msword";
        mimeTypes["wk1"] = "application/x-123";
        mimeTypes["wmf"] = "windows/metafile";
        mimeTypes["wml"] = "text/vnd.wap.wml";
        mimeTypes["wmlc"] = "application/vnd.wap.wmlc";
        mimeTypes["wmls"] = "text/vnd.wap.wmlscript";
        mimeTypes["wmlsc"] = "application/vnd.wap.wmlscriptc";
        mimeTypes["word"] = "application/msword";
        mimeTypes["wp"] = "application/wordperfect";
        mimeTypes["wp5"] = "application/wordperfect";
        mimeTypes["wp5"] = "application/wordperfect6.0";
        mimeTypes["wp6"] = "application/wordperfect";
        mimeTypes["wpd"] = "application/wordperfect";
        mimeTypes["wpd"] = "application/x-wpwin";
        mimeTypes["wq1"] = "application/x-lotus";
        mimeTypes["wri"] = "application/mswrite";
        mimeTypes["wri"] = "application/x-wri";
        mimeTypes["wrl"] = "application/x-world";
        mimeTypes["wrl"] = "model/vrml";
        mimeTypes["wrl"] = "x-world/x-vrml";
        mimeTypes["wrz"] = "model/vrml";
        mimeTypes["wrz"] = "x-world/x-vrml";
        mimeTypes["wsc"] = "text/scriplet";
        mimeTypes["wsrc"] = "application/x-wais-source";
        mimeTypes["wtk"] = "application/x-wintalk";
        mimeTypes["xbm"] = "image/x-xbitmap";
        mimeTypes["xbm"] = "image/x-xbm";
        mimeTypes["xbm"] = "image/xbm";
        mimeTypes["xdr"] = "video/x-amt-demorun";
        mimeTypes["xgz"] = "xgl/drawing";
        mimeTypes["xif"] = "image/vnd.xiff";
        mimeTypes["xl"] = "application/excel";
        mimeTypes["xla"] = "application/excel";
        mimeTypes["xla"] = "application/x-excel";
        mimeTypes["xla"] = "application/x-msexcel";
        mimeTypes["xlb"] = "application/excel";
        mimeTypes["xlb"] = "application/vnd.ms-excel";
        mimeTypes["xlb"] = "application/x-excel";
        mimeTypes["xlc"] = "application/excel";
        mimeTypes["xlc"] = "application/vnd.ms-excel";
        mimeTypes["xlc"] = "application/x-excel";
        mimeTypes["xld"] = "application/excel";
        mimeTypes["xld"] = "application/x-excel";
        mimeTypes["xlk"] = "application/excel";
        mimeTypes["xlk"] = "application/x-excel";
        mimeTypes["xll"] = "application/excel";
        mimeTypes["xll"] = "application/vnd.ms-excel";
        mimeTypes["xll"] = "application/x-excel";
        mimeTypes["xlm"] = "application/excel";
        mimeTypes["xlm"] = "application/vnd.ms-excel";
        mimeTypes["xlm"] = "application/x-excel";
        mimeTypes["xls"] = "application/excel";
        mimeTypes["xls"] = "application/vnd.ms-excel";
        mimeTypes["xls"] = "application/x-excel";
        mimeTypes["xls"] = "application/x-msexcel";
        mimeTypes["xlt"] = "application/excel";
        mimeTypes["xlt"] = "application/x-excel";
        mimeTypes["xlv"] = "application/excel";
        mimeTypes["xlv"] = "application/x-excel";
        mimeTypes["xlw"] = "application/excel";
        mimeTypes["xlw"] = "application/vnd.ms-excel";
        mimeTypes["xlw"] = "application/x-excel";
        mimeTypes["xlw"] = "application/x-msexcel";
        mimeTypes["xm"] = "audio/xm";
        mimeTypes["xml"] = "application/xml";
        mimeTypes["xml"] = "text/xml";
        mimeTypes["xmz"] = "xgl/movie";
        mimeTypes["xpix"] = "application/x-vnd.ls-xpix";
        mimeTypes["xpm"] = "image/x-xpixmap";
        mimeTypes["xpm"] = "image/xpm";
        mimeTypes["x-png"] = "image/png";
        mimeTypes["xsr"] = "video/x-amt-showrun";
        mimeTypes["xwd"] = "image/x-xwd";
        mimeTypes["xwd"] = "image/x-xwindowdump";
        mimeTypes["xyz"] = "chemical/x-pdb";
        mimeTypes["z"] = "application/x-compress";
        mimeTypes["z"] = "application/x-compressed";
        mimeTypes["zip"] = "application/x-compressed";
        mimeTypes["zip"] = "application/x-zip-compressed";
        mimeTypes["zip"] = "application/zip";
        mimeTypes["zip"] = "multipart/x-zip";
        mimeTypes["zoo"] = "application/octet-stream";
        mimeTypes["zsh"] = "text/x-script.zsh";
    }

    void before_handle(crow::request &req, crow::response &res, context &ctx) {
        try {
            const char *staticName = "/static/";
            auto isStaticFile = strncmp(req.url.c_str(), staticName, strlen(staticName)) == 0;
            if (isStaticFile) {
                size_t filenameLength = req.url.size() - strlen(staticName);
                char *filePath = new char[filenameLength];
                strcpy(filePath, req.url.c_str() + strlen(staticName));

                char *fullFilePath = new char[strlen(filePath) + strlen(StaticFolder)];

                strcat(fullFilePath, StaticFolder);
                strcat(fullFilePath, "/");
                strcat(fullFilePath, filePath);

                if (fileExists(fullFilePath)) {

                    try {
                        res.code = 200;

                        std::ifstream ifs(fullFilePath);
                        string str(static_cast<stringstream const &>(stringstream() << ifs.rdbuf()).str());
                        ifs.close();

                        auto dotLocation = this->indexOf(filePath, '.') + 1;
                        char *fileType = new char[filenameLength - dotLocation];
                        strcpy(fileType, filePath + dotLocation);

                        if (mimeTypes.find(fileType) != mimeTypes.end())
                            res.add_header("Content-Type", mimeTypes.find(fileType)->second);

                        res.write(str);
                    } catch (std::exception &e) {
                        res.clear();
                        res.code = 500;
                        res.write("Internal Server Error");
                    }
                }
                else
                    res.code = 404;

                res.end();
            }
        }
        catch (std::exception &e) {
            cout << e.what() << endl;
        }
    }

    void after_handle(crow::request & /*req*/, crow::response &res, context &ctx) {
        res.add_header("Server", "Native C++ Server");
    }

private:

    long getFileSize(FILE *file) {
        long lCurPos, lEndPos;
        lCurPos = ftell(file);
        fseek(file, 0, 2);
        lEndPos = ftell(file);
        fseek(file, lCurPos, 0);
        return lEndPos;

    }

    bool fileExists(string const &filename) {
        if (FILE *file = fopen(filename.c_str(), "r")) {
            fclose(file);
            return true;
        }

        return false;
    }

    size_t indexOf(char *text, char searchChar) {
        size_t totalLength = strlen(text);
        char *e = text + totalLength;
        size_t idx = 0;
        while (*e-- != searchChar && totalLength != idx)
            idx++;

        return totalLength - idx;
    }

    CUSTOM_MAP<string, string> mimeTypes;
};

char *StaticFileParser::StaticFolder = (char *) "";

int main(int argc, char **args) {

    cout << args[0] << endl;
    StaticFileParser::StaticFolder = "/Users/erhanbaris/ClionProjects/RecommanderAPI/www/static";
    cout << " --- MACHINE LEARNING SERVER ---" << endl << endl;

    try {
        //dataSource = new core::data::CvsDataSource("C:\\Users\\ErhanBARIS\\Downloads\\ml-latest-small\\movies.csv", "C:\\Users\\ErhanBARIS\\Downloads\\ml-latest-small\\ratings.csv");
        dataSource = new core::data::CvsDataSource<core::data::DataInfo>(
                "/Users/erhanbaris/Downloads/ml-20m/movies.csv", "/Users/erhanbaris/Downloads/ml-20m/ratings.csv");
        //dataSource = new core::data::MongoDbDataSource("mongodb://localhost:27017");
        dataSource->LoadData();

        cout << "Data Load Success" << endl;

        mDistance.SetProductIndex(&dataSource->Data()->productMap);
        mDistance.SetUserIndex(&dataSource->Data()->userMap);

        crow::App<crow::CookieParser, StaticFileParser> app;

        CROW_ROUTE(app, "/")
                ([]() {
                    std::ifstream ifs(("/Users/erhanbaris/ClionProjects/RecommanderAPI/www/index.html"));
                    string str(static_cast<stringstream const &>(stringstream() << ifs.rdbuf()).str());
                    ifs.close();
                    return str;
                });

        CROW_ROUTE(app, "/static/<string>")
                ([](string str) {

                    cout << str << endl;
                    return "<b>/api/recommend</b><br /><b>/system/refresh</b>";
                });


        CROW_ROUTE(app, "/system/refresh")
                ([]() {
                    dataSource->LoadData();
                    return "OK";
                });

        CROW_ROUTE(app, "/api/recommend/<int>")
                ([](USER_TYPE userId) {
                    vector<pair<PRODUCT_TYPE, wstring>> results;

#ifdef ENABLE_CACHE
                    auto cacheCheck = dataSource->Data()->recommendCacheForUser.find(userId);
    if (cacheCheck != dataSource->Data()->recommendCacheForUser.end())
        results = cacheCheck->second;
    else {
#endif
                    results = recommend(userId);
                    dataSource->Data()->recommendCacheForUser[userId] = results;
#ifdef ENABLE_CACHE
                    }
#endif

                    std::stringstream stream;
                    stream << "[";

                    auto end = results.cend();
                    for (auto it = results.cbegin(); it != end; ++it) {
                        string tmpName;
                        tmpName.assign(it->second.begin(), it->second.end());

                        stream << "{\"Id\":" << it->first << ",\"Name\":\"" << escapeJsonString(tmpName) << "\"},";
                    }

                    stream.seekg(0, ios::end);
                    int size = stream.tellg();

                    if (size > 1)
                        stream.seekp(-1, stream.cur);

                    stream << "]";
                    crow::response res(stream.str());
                    res.set_header("Content-Type", "application/json");
                    return res;
                });

        app.port(HTTP_SERVER_PORT).multithreaded().run();

    }
    catch (std::exception &e) {
        cout << "[ FATAL ERROR ] : " << e.what() << endl;
    }

    getchar();
    cout << "get chaa" << endl;
    return 0;
}

