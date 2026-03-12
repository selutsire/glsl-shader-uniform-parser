#pragma once

#include <iostream>
#include <unordered_set>
#include <string>
#include "misc/macros.h"

class FileParser{
    public:
   
    /*
    Take index into raw string, find index of the beginning of the next token and its end.
    Takes: char* for string to analyze, index into current position to analyze from, reference to where to place the 
    index into the string for the beginning and end of the tokens.
    */
    static void getNextWord(const char* string, 
                            unsigned int index,
                            int& beginTokenHolder,
                            int& endTokenHolder,
                            int& braceDepthHolder,
                            int& parenthesisDepthHolder,
                            bool& newExpressionHolder,
                            bool& equalDetectedHolder,
                            bool excludeComments = false);
    /*
    Takes utf 8 character and returns whether its a letter/number or another character
    */
    static bool isLetterOrNum(char character);
    /*
    Parses through GLSL code and adds any uniforms found inside to the vector given to it. 
    Takes: a string representing source code and a reference to a vector of strings that will contain all the uniforms.
    */
    static void parseGLSLUniforms(const std::string& sourceCode, std::vector<std::string>& uniformVector);
    private:
    FileParser() {}
};

    const std::unordered_set<std::string> glslkeywords = {
        "attribute", "const", "uniform", "varying", "layout", "in", "out", 
        "inout", "break", "continue", "discard", "return", "void", "struct", 
        "vec2", "vec3", "vec4", "mat2", "mat3", "mat4", "float", 
        "int", "bool", "bvec2", "bvec3", "bvec4", "ivec2", "ivec3", "ivec4", 
        "uvec2", "uvec3", "uvec4", "mat2x2", "mat2x3", "mat3x2", "mat3x3",
        "mat3x4", "mat4x2", "mat4x3", "mat4x4", "sampler1D", "sampler2D", 
        "sampler3D", "samplerCube", "sampler1DShadow", "sampler2DShadow", 
        "samplerCubeShadow", "sampler1DArray", "sampler2DArray", 
        "sampler1DArrayShadow", "sampler2DArrayShadow", "samplerCubeArray", 
        "samplerCubeArrayShadow", "sampler2DMS", "sampler2DMSArray", 
        "samplerBuffer", "sampler2DRect", "sampler2DRectShadow", "buffer",
        "invariant", "centroid", "flat", "smooth", "noperspective", "if", 
        "else", "while", "do", "for", "switch", "case", "default", "highp", 
        "mediump", "lowp", "precision", "subroutine", "common", "partition", 
        "active", "asm", "class", "union", "enum", "typedef", "template", 
        "this", "packed", "goto", "uint", "double", "dvec2", "dvec3", "dvec4",
        "dmat2", "dmat3", "dmat4",
        "dmat2x2", "dmat2x3", "dmat2x4",
        "dmat3x2", "dmat3x3", "dmat3x4",
        "dmat4x2", "dmat4x3", "dmat4x4",
        // integer samplers
        "isampler1D", "isampler2D", "isampler3D", "isamplerCube",
        "isampler1DArray", "isampler2DArray", "isamplerBuffer",
        "usampler1D", "usampler2D", "usampler3D", "usamplerCube",
        "usampler1DArray", "usampler2DArray", "usamplerBuffer",
        // atomic and image types
        "atomic_uint",
        "image2D", "image3D", "imageCube", "image2DArray",
        "iimage2D", "uimage2D",
        // other qualifiers
        "coherent", "volatile", "restrict", "readonly", "writeonly",
        "shared", // compute shaders
        "patch",  // tessellation
        // built in variables people sometimes name things after
        "gl_Position", "gl_FragCoord", "gl_FragDepth",
        //memory layout qualifiers
        "std140", "std430", "packed", "shared", "location", "binding", 
    };