//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//
//
//===========================================================================================================================
// PathTracks
//============================================================================================

#include "path_tracks.h"

#include <libs/core/core.h>
#include <libs/filesystem/v_file_service.h>

#include "camera_tracks_file.h"

//============================================================================================

PathTracks::PathTracks()
{
    point     = nullptr;
    numPoints = 0;
}

PathTracks::~PathTracks()
{
    delete point;
    point     = nullptr;
    numPoints = 0;
}

//============================================================================================

// Upload track to PathTracks
bool PathTracks::Load(char const* fileName)
{
    // Loading the file into memory
    Assert(sizeof(AntFileTrackElement) == sizeof(Point));

    delete point;
    point     = nullptr;
    numPoints = 0;

    std::vector<char> data = {};
    if (!fio->read_file_to_mem(fileName, data) || data.empty()) {
        core->Trace("Camera tracks file %s not loaded...", fileName);
        return false;
    }

    // Checking the title
    if (reinterpret_cast<AntFileHeader*>(data.data())->id != ANTFILE_ID) {
        core->Trace("Camera tracks file %s is invalidate...", fileName);
        return false;
    }

    if (reinterpret_cast<AntFileHeader*>(data.data())->ver != ANTFILE_VER) {
        core->Trace("Camera tracks file %s have incorrect version...", fileName);
        return false;
    }

    int32_t const nPoints     = reinterpret_cast<AntFileHeader*>(data.data())->framesCount;
    int32_t const nStringSize = reinterpret_cast<AntFileHeader*>(data.data())->stringsTableSize;
    int32_t const nBoneCount  = reinterpret_cast<AntFileHeader*>(data.data())->bonesCount;

    // Checking file sizes
    if (data.size()
        < (sizeof(AntFileHeader) + (sizeof(char) * nStringSize) + (sizeof(AntFileBone) * nBoneCount)
           + (sizeof(AntFileTrackElement) * nPoints))) {
        core->Trace("Camera tracks file %s is invalidate...", fileName);
        return false;
    }

    // save the data
    point = new Point[nPoints];
    Assert(point);
    memcpy(
        point,
        data.data() + sizeof(AntFileHeader) + nStringSize + (sizeof(AntFileBone) * nBoneCount),
        sizeof(AntFileTrackElement) * nPoints);
    numPoints = nPoints;

    return true;
}

// Draw track
void PathTracks::Draw(/*VDX9RENDER*/ void* render) {}

// Get track point
bool PathTracks::GetPoint(float index, Vector& cp, Quaternion& cq) const
{
    Assert(point);
    if (index < 0.0 || index >= 1.f) return false;
    index *= numPoints;
    auto i1 = static_cast<int32_t>(index);
    auto i2 = i1 + 1;
    if (i1 >= numPoints) i1 = numPoints - 1;
    if (i2 >= numPoints) i2 = numPoints - 1;
    cp.Lerp(point[i1].p, point[i2].p, index - i1);
    cq.SLerp(point[i1].q, point[i2].q, index - i1);
    return true;
}
