/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmDepends.h"

#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"

#include <assert.h>

//----------------------------------------------------------------------------
cmDepends::cmDepends()
{
  m_Verbose = false;
}

//----------------------------------------------------------------------------
cmDepends::~cmDepends()
{
}

void cmDepends::SetTargetFile(const char* dir, const char* targetFile, 
                              const char *markExt, const char *makeExt)
{
  m_Directory = dir;
  m_TargetFile = targetFile;
  
  // Construct the path to the make and mark files.  Append
  // appropriate extensions to their names.
  m_DependsMarkFile = dir;
  m_DependsMakeFile = dir;
  m_DependsMakeFile += "/";
  m_DependsMarkFile += "/";
  m_DependsMakeFile += m_TargetFile;
  m_DependsMarkFile += m_TargetFile;
  m_DependsMakeFile += makeExt;
  m_DependsMarkFile += markExt;

  if (!m_CompileDirectory.size())
    {
    m_CompileDirectory = dir;
    }
}


//----------------------------------------------------------------------------
bool cmDepends::Write()
{
  // Dependency generation must always be done in the current working
  // directory.
  assert(m_Directory == ".");

  // Try to generate dependencies for the target file.
  cmGeneratedFileStream fout(m_DependsMakeFile.c_str());
  fout << "# Dependencies for " << m_TargetFile.c_str() << std::endl;
  if(this->WriteDependencies(fout) && fout)
    {
    // Dependencies were generated.  Touch the mark file.
    std::ofstream fmark(m_DependsMarkFile.c_str());
    fmark << "Dependencies updated for " << m_TargetFile.c_str() << std::endl;
    return true;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
void cmDepends::Check()
{
  // Dependency checks must be done in proper working directory.
  std::string oldcwd = ".";
  if(m_CompileDirectory != ".")
    {
    // Get the CWD but do not call CollapseFullPath because
    // we only need it to cd back, and the form does not matter
    oldcwd = cmSystemTools::GetCurrentWorkingDirectory(false);
    cmSystemTools::ChangeDirectory(m_CompileDirectory.c_str());
    }

  // Check whether dependencies must be regenerated.
  std::ifstream fin(m_DependsMakeFile.c_str());
  if(!(fin && this->CheckDependencies(fin)))
    {
    // Clear all dependencies so they will be regenerated.
    this->Clear();
    }

  // Restore working directory.
  if(oldcwd != ".")
    {
    cmSystemTools::ChangeDirectory(oldcwd.c_str());
    }
}

//----------------------------------------------------------------------------
void cmDepends::Clear()
{
  // Print verbose output.
  if(m_Verbose)
    {
    cmOStringStream msg;
    msg << "Clearing dependencies for \"" << m_TargetFile << "\"." << std::endl;
    cmSystemTools::Stdout(msg.str().c_str());
    }

  // Remove the dependency mark file to be sure dependencies will be
  // regenerated.
  cmSystemTools::RemoveFile(m_DependsMarkFile.c_str());

  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(m_DependsMakeFile.c_str());
  depFileStream
    << "# Empty dependencies file for " << m_TargetFile.c_str() << ".\n"
    << "# This may be replaced when dependencies are built." << std::endl;
}

//----------------------------------------------------------------------------
const char* cmDepends::GetMakeFileName()
{
  // Skip over the directory part of the name.
  return m_DependsMakeFile.c_str() + m_Directory.length() + 1;
}

//----------------------------------------------------------------------------
const char* cmDepends::GetMarkFileName()
{
  // Skip over the directory part of the name.
  return m_DependsMarkFile.c_str() + m_Directory.length() + 1;
}
