#pragma once
#include <sass.h>
#include <crow/logging.h>
#include <string>
#include <fstream>
#include <stdexcept>

namespace Web::Scss {
    inline void compile(const std::string& inputPath, const std::string& outputPath) {
        CROW_LOG_INFO << "Compiling SCSS: " << inputPath;

        Sass_File_Context* fileContext = sass_make_file_context(inputPath.c_str());
        Sass_Context* context = sass_file_context_get_context(fileContext);
        Sass_Options* options = sass_context_get_options(context);
        sass_option_set_output_style(options, SASS_STYLE_COMPRESSED);

        int error = sass_compile_file_context(fileContext);
        if (error) {
            std::string message = sass_context_get_error_message(context);
            sass_delete_file_context(fileContext);
            throw std::runtime_error(message);
        }

        std::ofstream output(outputPath);
        output << sass_context_get_output_string(context);
        sass_delete_file_context(fileContext);

        CROW_LOG_INFO << "SCSS compiled to: " << outputPath;
    }
}