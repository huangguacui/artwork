/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Christoph Kreisl [2018]
*/

#include <QGuiApplication>
#include <include/core/parser.h>
#include <include/core/proplist.h>
#include <include/renderer/artworkcolor.h>
#include <include/renderer/artworkgrey.h>
#include <include/renderer/renderitem.h>
#include <include/artworks/words.h>
#include <include/artworks/circles.h>
#include <include/artworks/triangles.h>
#include <include/artworks/rectangle.h>
#include <include/core/common.h>
#include <iostream>

#define ARTWORK_VERSION "0.0.1"
#define ARTWORK_YEAR "2018"

using namespace std;

void help() {
    cout << "Artwork version " << ARTWORK_VERSION
         << ", Copyright (c) " << ARTWORK_YEAR << " Christoph Kreisl" << endl;
    cout << "Usage: artwork [options] <rendersettings.xml>" << endl;
    cout << "Options/Arguments: " << endl;
    cout << "   -h --help   Display this help text" << endl;
    cout << "   -i iname    Load the input image from a path denoted by \"iname\"" << endl;
    cout << "   -o oname    Write the output image with the name denoted by \"oname\"" << endl;
    cout << "               If there is a file type it will be removed. Set output file type in XML rendersettings!" << endl;
    cout << "For more information check https://github.com/ckreisl" << endl;
}

int app_artwork(int argc, char *argv[]) {
    try {

        if(argc < 2) {
            help();
            return 0;
        }

        QString xmlFilePath = "";
        QString inputImgPath = "";
        QString outputName = "";

        std::vector<std::string> options;
        for(int i = 1; i < argc; ++i)
            options.push_back(argv[i]);

        for(unsigned int i = 0; i < options.size()-1; ++i) {
            if(options[i] == "-i") {
                inputImgPath = QString::fromUtf8(options[++i].c_str());
                if(!checkImageType(inputImgPath)) {
                    std::cerr << "Non or Wrong filetype given, load one of .jpg/.JPG/.png/.PNG/.jpeg/.JPEG" << std::endl;
                    return 0;
                }
            } else if(options[i] == "-o") {
                outputName = QString::fromUtf8(options[++i].c_str());
            } else if(options[i] == "-h" || options[i] == "--help") {
                help();
                return 0;
            }
        }

        if(options[options.size()-1] == "-h" || options[options.size()-1] == "--help") {
            help();
            return 0;
        }

        xmlFilePath = QString::fromUtf8(options[options.size()-1].c_str());

        if(xmlFilePath == "" || xmlFilePath.right(4) != ".xml") {
            std::cerr << "No XML file was specified or file is not a XML file!" << std::endl;
            help();
            return 0;
        }

        PropertyList *props = loadFromXML(xmlFilePath);
        if(!props) {
            cerr << "Failed to load Properties from XML file" << endl;
            return 0;
        }

        /* set user settings to property list*/
        if(QString::compare(inputImgPath, "") != 0)
            props->setString("userCommandLineInputImg", inputImgPath);
        if(QString::compare(outputName, "") != 0)
            props->setString("userCommandLineOutputName", outputName);

        /* set up image rendering process */
        bool colorMode = props->getBoolean("colorMode");
        bool renderWords = props->getBoolean("renderWords");
        bool renderTriangle = props->getBoolean("renderTriangles");
        bool renderCircles = props->getBoolean("renderCircles");
        bool renderRectangles = props->getBoolean("renderRectangles");

        if((QString::compare(outputName, "", Qt::CaseInsensitive) != 0) &&
                ((renderWords && renderTriangle) ||
                 (renderCircles && renderRectangles) ||
                 (renderWords && renderCircles) ||
                 (renderTriangle && renderCircles) ||
                 (renderRectangles && renderWords) ||
                 (renderRectangles && renderCircles) ||
                 (renderRectangles && renderTriangle))) {
            cout << "****************************************************************************************" << endl;
            cout << "* NOTE: Multiple ArtWorks renderer are active and an output name was specified.        *" << endl;
            cout << "* This will override your result image everytime an artwork renderer finishes his job! *" << endl;
            cout << "****************************************************************************************" << endl;
        } else if((!renderWords) && (!renderTriangle) && (!renderCircles) && (!renderRectangles)) {
            cout << "Enable one render option in your render settings!" << endl;
        }

        std::vector<ArtWorkColor *> thrListColor;
        std::vector<ArtWorkGrey *> thrListGrey;
        if(renderWords) {
            RenderItem *renderItem = new Words(*props);
            if(colorMode) {
                ArtWorkColor *wp = new ArtWorkColor(*props, renderItem);
                wp->start();
                thrListColor.push_back(wp);
            } else {
                ArtWorkGrey *wp = new ArtWorkGrey(*props, renderItem);
                wp->start();
                thrListGrey.push_back(wp);
            }
        }

        if(renderTriangle) {
            RenderItem *renderItem = new Triangles(*props);
            if(colorMode) {
                ArtWorkColor *tp = new ArtWorkColor(*props, renderItem);
                tp->start();
                thrListColor.push_back(tp);
            } else {
                ArtWorkGrey *tp = new ArtWorkGrey(*props, renderItem);
                tp->start();
                thrListGrey.push_back(tp);
            }
        }

        if(renderCircles) {
            RenderItem *renderItem = new Circles(*props);
            if(colorMode) {
                ArtWorkColor *cp = new ArtWorkColor(*props, renderItem);
                cp->start();
                thrListColor.push_back(cp);
            } else {
                ArtWorkGrey *cp = new ArtWorkGrey(*props, renderItem);
                cp->start();
                thrListGrey.push_back(cp);
            }
        }

        if(renderRectangles) {
            RenderItem *renderItem = new Rectangle(*props);
            if(colorMode) {
                ArtWorkColor *cp = new ArtWorkColor(*props, renderItem);
                cp->start();
                thrListColor.push_back(cp);
            } else {
                ArtWorkGrey *cp = new ArtWorkGrey(*props, renderItem);
                cp->start();
                thrListGrey.push_back(cp);
            }
        }

        /* shutdown all rendering processes and clean up */
        for(ArtWorkColor *ac : thrListColor) {
            ac->wait();
            delete ac;
        }

        for(ArtWorkGrey *ag : thrListGrey) {
            ag->wait();
            delete ag;
        }

        delete props;
        return 0;

    } catch (std::exception e) {
        std::cerr << "Error occured" << e.what() << std::endl;
        return -1;
    }
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    int retVal = app_artwork(argc, argv);

    app.exit(retVal);
}
