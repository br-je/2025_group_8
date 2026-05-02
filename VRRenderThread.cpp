//This is the adaptation of the VRTK Demo program.
//It currently works using the "Start VR button" in the main window
#include "VRRenderThread.h"

#include <vtkActor.h>
#include <vtkOpenVRCamera.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkLight.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>

#include <array>

//These includes have been used to scale the objects in the scene
#include <algorithm>
#include <limits>
#include <cmath>

//For skybox and floor aesthetics
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTexture.h>
#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>

//Live updating VR method https://doc.qt.io/qt-6/qmutexlocker.html (Experimental, but prevents crashing currently)
#include <QMutexLocker>


VRRenderThread::VRRenderThread(QObject* parent)
    : QThread(parent)
{
}

//This code is currently considered experimental
//Values should be changed to perfectly fit the VR scene to scale the STL properly
//This function allows actors to be added to the VR scene both before and during VR runtime 
//EDIT THIS FUNCTION IF CRASHES OCCUR WHEN ADDING ACTORS DURING VR RUNTIME
void VRRenderThread::addActorOffline(vtkSmartPointer<vtkActor> actor)
{
    if (!actor)
    {
        return;
    }

    if (!this->isRunning())
    {
        // Before VR starts, actors can be added directly to the starting list.
        actors.append(actor);
    }
    else
    {
        // While VR is running, queue actors so the VR thread can add them safely.
        QMutexLocker locker(&pendingActorsMutex);
        pendingActors.append(actor);
    }
}

void VRRenderThread::toggleExplode()
{
    explodeToggleRequested = true;
}

void VRRenderThread::queueVRPipelineUpdate(ModelPart* part)
{
    if (!part)
        return;
    QMutexLocker locker(&pendingVRUpdatesMutex);
    if (!pendingVRUpdates.contains(part))
        pendingVRUpdates.append(part);
}

//Animation functions
void VRRenderThread::setAnimationEnabled(bool enabled)
{
    animationEnabled = enabled;
}
bool VRRenderThread::animationIsEnabled() const
{
    return animationEnabled;
}

void VRRenderThread::resetView()
{
    // Only request the reset here.
    // The VR thread will apply the actor transforms safely inside its render loop.
    resetRequested = true;
}

//This function signals the VR render thread to stop and performs necessary cleanup of VR resources.
void VRRenderThread::stopVR()
{
    // Do not directly call OpenVR/VTK cleanup from the GUI thread.
    // The VR thread will detect this flag and terminate itself safely.
    stopRequested = true;
}

// Fires when a controller button is pressed or released.
// Trigger press: find the nearest actor to the controller tip and grab it.
// Trigger release: drop it.
void VRRenderThread::OnButton3D(vtkObject* /*caller*/, unsigned long /*eventId*/,
                                 void* clientData, void* callData)
{
    auto* self = static_cast<VRRenderThread*>(clientData);
    auto* ed   = static_cast<vtkEventData*>(callData);
    auto* bd   = ed->GetAsEventDataButton3D();
    if (!bd)
        return;

    // Grip button toggles the explode animation.
    if (bd->GetInput() == vtkEventDataDeviceInput::Grip &&
        bd->GetAction() == vtkEventDataAction::Press)
    {
        self->toggleExplode();
        return;
    }

    if (bd->GetInput() != vtkEventDataDeviceInput::Trigger)
        return;

    double pos[3];
    bd->GetWorldPosition(pos);

    if (bd->GetAction() == vtkEventDataAction::Press)
    {
        // Find the actor whose centre is closest to the controller tip,
        // within a 25 cm grab radius. Using squared distance avoids sqrt.
        const double grabRadiusSq = 0.25 * 0.25;
        vtkSmartPointer<vtkActor> closest;
        double minDistSq = grabRadiusSq;

        for (auto& actor : self->actors)
        {
            if (!actor)
                continue;

            double bounds[6];
            actor->GetBounds(bounds);

            double cx = (bounds[0] + bounds[1]) * 0.5;
            double cy = (bounds[2] + bounds[3]) * 0.5;
            double cz = (bounds[4] + bounds[5]) * 0.5;

            double dx = pos[0] - cx;
            double dy = pos[1] - cy;
            double dz = pos[2] - cz;
            double distSq = dx*dx + dy*dy + dz*dz;

            if (distSq < minDistSq)
            {
                minDistSq = distSq;
                closest   = actor;
            }
        }

        // If this actor was hovered (highlighted), restore its colour before grabbing.
        if (closest && self->hoveredActor == closest)
        {
            closest->GetProperty()->SetColor(
                self->hoveredOriginalColor[0],
                self->hoveredOriginalColor[1],
                self->hoveredOriginalColor[2]
            );
            closest->Modified();
            self->hoveredActor = nullptr;
        }

        self->draggedActor    = closest;
        self->grabbingDevice  = bd->GetDevice();
        self->lastControllerPos[0] = pos[0];
        self->lastControllerPos[1] = pos[1];
        self->lastControllerPos[2] = pos[2];
    }
    else if (bd->GetAction() == vtkEventDataAction::Release)
    {
        self->draggedActor   = nullptr;
        self->grabbingDevice = vtkEventDataDevice::Unknown;
    }
}

// Fires every time a tracked controller moves.
// Handles both actor dragging and hover highlighting.
void VRRenderThread::OnMove3D(vtkObject* /*caller*/, unsigned long /*eventId*/,
                               void* clientData, void* callData)
{
    auto* self = static_cast<VRRenderThread*>(clientData);

    auto* ed = static_cast<vtkEventData*>(callData);
    auto* md = ed->GetAsEventDataDevice3D();
    if (!md)
        return;

    double pos[3];
    md->GetWorldPosition(pos);

    // --- Dragging ---
    if (self->draggedActor && md->GetDevice() == self->grabbingDevice)
    {
        double* cur = self->draggedActor->GetPosition();
        self->draggedActor->SetPosition(
            cur[0] + pos[0] - self->lastControllerPos[0],
            cur[1] + pos[1] - self->lastControllerPos[1],
            cur[2] + pos[2] - self->lastControllerPos[2]
        );
        self->draggedActor->Modified();

        self->lastControllerPos[0] = pos[0];
        self->lastControllerPos[1] = pos[1];
        self->lastControllerPos[2] = pos[2];
        return;
    }

    // --- Hover highlight (only when nothing is grabbed) ---
    const double grabRadiusSq = 0.25 * 0.25;
    vtkSmartPointer<vtkActor> nearest;
    double minDistSq = grabRadiusSq;

    for (auto& actor : self->actors)
    {
        if (!actor)
            continue;

        double bounds[6];
        actor->GetBounds(bounds);

        double cx = (bounds[0] + bounds[1]) * 0.5;
        double cy = (bounds[2] + bounds[3]) * 0.5;
        double cz = (bounds[4] + bounds[5]) * 0.5;

        double dx = pos[0] - cx;
        double dy = pos[1] - cy;
        double dz = pos[2] - cz;
        double distSq = dx*dx + dy*dy + dz*dz;

        if (distSq < minDistSq)
        {
            minDistSq = distSq;
            nearest   = actor;
        }
    }

    // If the hovered actor has changed, restore the old one and highlight the new one.
    if (nearest != self->hoveredActor)
    {
        if (self->hoveredActor)
        {
            self->hoveredActor->GetProperty()->SetColor(
                self->hoveredOriginalColor[0],
                self->hoveredOriginalColor[1],
                self->hoveredOriginalColor[2]
            );
            self->hoveredActor->Modified();
        }

        self->hoveredActor = nearest;

        if (nearest)
        {
            // Save the part's real colour then apply a bright yellow highlight.
            nearest->GetProperty()->GetColor(self->hoveredOriginalColor);
            nearest->GetProperty()->SetColor(1.0, 1.0, 0.0);
            nearest->Modified();
        }
    }
}

/**
This function initialises the OpenVR renderer, adds all pre-prepared actors, and continuously updates the VR scene until stopped.
 Key steps:
 - Create VR renderer and interactor
 - Add all actors generated from ModelParts
 - Compute a global bounding box to correctly scale and centre the full CAD assembly
 - Apply consistent transformations to preserve relative positioning of parts
 - Enter render loop to allow real-time interaction

 The thread runs independently from the Qt GUI to ensure responsive VR rendering.
*/
void VRRenderThread::run()
{

    stopRequested = false;

    vtkNew<vtkNamedColors> colors;

    std::array<unsigned char, 4> bkg{ {26, 51, 102, 255} };
    colors->SetColor("BkgColor", bkg.data());

    vtkNew<vtkOpenVRRenderer> renderer;

    // Add improved lighting for the VR scene.
    // A headlight keeps the model visible from the user's viewpoint.
    vtkNew<vtkLight> headLight;
    headLight->SetLightTypeToHeadlight();
    headLight->SetIntensity(0.6);
    renderer->AddLight(headLight);

    // A scene light adds stronger directional lighting and depth.
    // This follows the lighting approach suggested in the group task sheet.
    vtkNew<vtkLight> sceneLight;
    sceneLight->SetLightTypeToSceneLight();
    sceneLight->SetPosition(5.0, 5.0, 15.0);
    sceneLight->SetPositional(true);
    sceneLight->SetConeAngle(30.0);
    sceneLight->SetFocalPoint(0.0, 0.0, -2.0);
    sceneLight->SetDiffuseColor(1.0, 1.0, 1.0);
    sceneLight->SetAmbientColor(0.6, 0.6, 0.6);
    sceneLight->SetSpecularColor(1.0, 1.0, 1.0);
    sceneLight->SetIntensity(0.8);
    renderer->AddLight(sceneLight);

    for (auto actor : actors)
    {
        if (actor)
        {
            renderer->AddActor(actor);
        }
    }

    // Calculate one global bounding box for the whole CAD assembly
    double globalBounds[6] = {
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest()
    };

    bool hasBounds = false;

    for (auto actor : actors)
    {
        if (!actor)
        {
            continue;
        }
        double bounds[6];
        actor->GetBounds(bounds);
        if (bounds[0] <= bounds[1] &&
            bounds[2] <= bounds[3] &&
            bounds[4] <= bounds[5])
        {
            globalBounds[0] = std::min(globalBounds[0], bounds[0]);
            globalBounds[1] = std::max(globalBounds[1], bounds[1]);
            globalBounds[2] = std::min(globalBounds[2], bounds[2]);
            globalBounds[3] = std::max(globalBounds[3], bounds[3]);
            globalBounds[4] = std::min(globalBounds[4], bounds[4]);
            globalBounds[5] = std::max(globalBounds[5], bounds[5]);

            hasBounds = true;
        }
    }

    double vrCentreX = 0.0;
    double vrCentreY = 0.0;
    double vrZMin    = 0.0;
    double vrScale   = 1.0;
    bool hasVRTransform = false;

    if (hasBounds)
    {
        double centreX = (globalBounds[0] + globalBounds[1]) / 2.0;
        double centreY = (globalBounds[2] + globalBounds[3]) / 2.0;

        double sizeX = globalBounds[1] - globalBounds[0];
        double sizeY = globalBounds[3] - globalBounds[2];
        double sizeZ = globalBounds[5] - globalBounds[4];

        // The lowest Z across the whole assembly — used to place the model
        // so its base sits exactly on the floor (world Y = 0).
        double zMin = globalBounds[4];

        double maxSize = std::max(sizeX, std::max(sizeY, sizeZ));
        if (maxSize > 0.0)
        {
            // 0.6 gives a comfortable viewing size — the longest axis of the
            // assembly becomes 0.6m, which fits fully in view at 2m distance.
            double scale = 0.6 / maxSize;

            vrCentreX = centreX;
            vrCentreY = centreY;
            vrZMin    = zMin;
            vrScale   = scale;
            hasVRTransform = true;

            for (auto actor : actors)
            {
                if (!actor)
                {
                    continue;
                }

                actor->SetScale(scale, scale, scale);

                // RotateX(-90) converts CAD Z-up to VR Y-up.
                // After rotation: object-X -> world-X,
                //                 object-Z -> world-Y,
                //                 object-Y -> world -Z.
                // X: centre on world X=0 using centreX.
                // Y: use zMin (not centreZ) so the model base lands at Y=0,
                //    not the centroid — otherwise the model is half-buried.
                // Z: use centreY so the assembly is centred in depth, then
                //    push 2m in front of the user.
                actor->RotateX(-90.0);
                actor->SetPosition(
                    -centreX * scale,
                    -zMin    * scale,
                     centreY * scale - 2.0
                );
            }
        }
    }

    // Store the initial VR transforms after scaling, positioning and orientation correction.
    // These are used by Reset View to undo animation/rotation changes.
    originalPositions.clear();
    originalScales.clear();
    originalOrientations.clear();

    for (auto actor : actors)
    {
        if (!actor)
        {
            continue;
        }

        double position[3];
        double scale[3];
        double orientation[3];

        actor->GetPosition(position);
        actor->GetScale(scale);
        actor->GetOrientation(orientation);

        originalPositions.append({ position[0], position[1], position[2] });
        originalScales.append({ scale[0], scale[1], scale[2] });
        originalOrientations.append({ orientation[0], orientation[1], orientation[2] });
    }

    // Compute exploded positions — each part moves outward from the assembly centroid.
    // Direction is from centroid to each part's bounds centre; distance is 0.3m.
    explodedPositions.clear();
    {
        // Find the world-space bounds centre of each actor (geometry centroid after transforms).
        QList<std::array<double, 3>> boundsCentres;
        for (auto actor : actors)
        {
            if (!actor) continue;
            double b[6];
            actor->GetBounds(b);
            boundsCentres.append({
                (b[0] + b[1]) * 0.5,
                (b[2] + b[3]) * 0.5,
                (b[4] + b[5]) * 0.5
            });
        }

        // Assembly centroid = average of all bounds centres.
        double cx = 0, cy = 0, cz = 0;
        for (auto& bc : boundsCentres) { cx += bc[0]; cy += bc[1]; cz += bc[2]; }
        int n = boundsCentres.size();
        if (n > 0) { cx /= n; cy /= n; cz /= n; }

        // Exploded position = original SetPosition + outward direction * 0.3m.
        for (int i = 0; i < actors.size() && i < (int)boundsCentres.size(); i++)
        {
            double dx = boundsCentres[i][0] - cx;
            double dy = boundsCentres[i][1] - cy;
            double dz = boundsCentres[i][2] - cz;
            double len = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (len < 0.001) { dx = 0; dy = 1; dz = 0; len = 1; }

            const double explodeDist = 0.3;
            explodedPositions.append({
                originalPositions[i][0] + (dx / len) * explodeDist,
                originalPositions[i][1] + (dy / len) * explodeDist,
                originalPositions[i][2] + (dz / len) * explodeDist
            });
        }
    }

    // Add a simple floor to make the VR environment more realistic.
    // This helps avoid the scene appearing as just a plain background colour.
	// Play around with the floor position and size to best fit the CAD assembly and improve depth perception in VR.
    vtkNew<vtkPlaneSource> floorSource;

    double floorY = 0.0;
    floorSource->SetOrigin(-5.0, floorY, -8.0);
    floorSource->SetPoint1(5.0, floorY, -8.0);
    floorSource->SetPoint2(-5.0, floorY, 4.0);
    floorSource->Update();

    vtkNew<vtkPolyDataMapper> floorMapper;
    floorMapper->SetInputConnection(floorSource->GetOutputPort());

    vtkNew<vtkActor> floorActor;
    floorActor->SetMapper(floorMapper);
    floorActor->GetProperty()->SetColor(0.35, 0.35, 0.35);
    floorActor->GetProperty()->SetOpacity(1.0);


    renderer->AddActor(floorActor);

    // Grid of lines sitting just above the floor under the car.
    // The car appears centred at X=0, Z=-2.0, so the grid is centred there.
    // Y=0.002 is a tiny offset above the floor to prevent z-fighting.
    {
        const double gridY      =  0.002;
        const double gridCentreZ = -2.0;
        const double halfExtent  =  1.5;   // grid reaches 1.5m either side of centre
        const double spacing     =  0.1;   // one line every 10cm

        const double xMin = -halfExtent;
        const double xMax =  halfExtent;
        const double zMin =  gridCentreZ - halfExtent;
        const double zMax =  gridCentreZ + halfExtent;

        vtkNew<vtkPolyData>  gridData;
        vtkNew<vtkPoints>    gridPoints;
        vtkNew<vtkCellArray> gridLines;

        // Lines running along Z (constant X)
        for (double x = xMin; x <= xMax + 1e-9; x += spacing)
        {
            vtkIdType p0 = gridPoints->InsertNextPoint(x, gridY, zMin);
            vtkIdType p1 = gridPoints->InsertNextPoint(x, gridY, zMax);
            vtkIdType pts[2] = { p0, p1 };
            gridLines->InsertNextCell(2, pts);
        }

        // Lines running along X (constant Z)
        for (double z = zMin; z <= zMax + 1e-9; z += spacing)
        {
            vtkIdType p0 = gridPoints->InsertNextPoint(xMin, gridY, z);
            vtkIdType p1 = gridPoints->InsertNextPoint(xMax, gridY, z);
            vtkIdType pts[2] = { p0, p1 };
            gridLines->InsertNextCell(2, pts);
        }

        gridData->SetPoints(gridPoints);
        gridData->SetLines(gridLines);

        vtkNew<vtkPolyDataMapper> gridMapper;
        gridMapper->SetInputData(gridData);

        vtkNew<vtkActor> gridActor;
        gridActor->SetMapper(gridMapper);
        gridActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
        gridActor->GetProperty()->SetLineWidth(1.0);
        gridActor->PickableOff();

        renderer->AddActor(gridActor);
    }

   // Add a simple environment sphere around the VR scene.
   // This acts as a lightweight skybox/scenery without relying on image files.
    vtkNew<vtkSphereSource> skySource;
    skySource->SetRadius(25.0);
    skySource->SetThetaResolution(64);
    skySource->SetPhiResolution(32);
    skySource->SetCenter(0.0, 0.0, -2.0);
    skySource->Update();

    vtkNew<vtkPolyDataMapper> skyMapper;
    skyMapper->SetInputConnection(skySource->GetOutputPort());

    vtkNew<vtkActor> skyActor;
    skyActor->SetMapper(skyMapper);

    // Make it look like a distant dark-blue VR environment.
    skyActor->GetProperty()->SetColor(0.04, 0.07, 0.14);
    skyActor->GetProperty()->SetAmbient(1.0);
    skyActor->GetProperty()->SetDiffuse(0.0);
    skyActor->GetProperty()->SetSpecular(0.0);

    // Prevent the sky from being selected/interacted with.
    skyActor->PickableOff();

    renderer->AddActor(skyActor);

	// Set a consistent background color for the VR scene
    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

	// Set up the VR camera and interactor
    vtkNew<vtkOpenVRCamera> cam;
    renderer->SetActiveCamera(cam);

    // Create the VR render window and interactor, and start the rendering loop
    vrRenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
    vrRenderWindow->Initialize();
    vrRenderWindow->AddRenderer(renderer);
    vrRenderWindow->SetWindowName("Group8 CAD VR");

    vrInteractor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
    vrInteractor->SetRenderWindow(vrRenderWindow);
    vrInteractor->Initialize();

    // Register controller callbacks for actor dragging.
    // These fire on the VR thread (inside ProcessEvents), so no cross-thread risk.
    vtkNew<vtkCallbackCommand> buttonCb;
    buttonCb->SetCallback(VRRenderThread::OnButton3D);
    buttonCb->SetClientData(this);
    vrInteractor->AddObserver(vtkCommand::Button3DEvent, buttonCb);

    vtkNew<vtkCallbackCommand> moveCb;
    moveCb->SetCallback(VRRenderThread::OnMove3D);
    moveCb->SetClientData(this);
    vrInteractor->AddObserver(vtkCommand::Move3DEvent, moveCb);

    // Run our own VR loop instead of using vrInteractor->Start().
    // This allows the Stop VR button to end the loop safely.
    while (!stopRequested)
    {
        if (vrInteractor)
        {
            vrInteractor->ProcessEvents();
        }

        // Add any STL actors that were loaded while VR is already running.
        // This is done inside the VR thread to avoid cross-thread VTK/OpenVR crashes.
        QList<vtkSmartPointer<vtkActor>> actorsToAdd;

        {
            QMutexLocker locker(&pendingActorsMutex);
            actorsToAdd = pendingActors;
            pendingActors.clear();
        }

        for (auto actor : actorsToAdd)
        {
            if (!actor)
            {
                continue;
            }

            // Apply the same VR transform as the original loaded assembly.
            if (hasVRTransform)
            {
                actor->SetScale(vrScale, vrScale, vrScale);
                actor->RotateX(-90.0);
                actor->SetPosition(
                    -vrCentreX * vrScale,
                    -vrZMin    * vrScale,
                     vrCentreY * vrScale - 2.0
                );
            }

            actors.append(actor);

            double position[3];
            double scale[3];
            double orientation[3];

            actor->GetPosition(position);
            actor->GetScale(scale);
            actor->GetOrientation(orientation);

            originalPositions.append({ position[0], position[1], position[2] });
            originalScales.append({ scale[0], scale[1], scale[2] });
            originalOrientations.append({ orientation[0], orientation[1], orientation[2] });

            renderer->AddActor(actor);
        }

        // Apply filter/property updates queued by the GUI thread.
        // Called here (inside VR thread) so VTK objects are only modified from one thread.
        {
            QList<ModelPart*> updatesToProcess;
            {
                QMutexLocker locker(&pendingVRUpdatesMutex);
                updatesToProcess = pendingVRUpdates;
                pendingVRUpdates.clear();
            }
            for (auto part : updatesToProcess)
            {
                if (part)
                    part->updateVRPipeline();
            }
        }

        // Explode toggle — flip direction when requested.
        if (explodeToggleRequested.exchange(false))
        {
            explodeActive = !explodeActive;
        }

        // Animate explode progress toward 1.0 (exploded) or 0.0 (assembled).
        {
            double target = explodeActive ? 1.0 : 0.0;
            if (explodeProgress != target)
            {
                explodeProgress += explodeActive ? 0.02 : -0.02;
                explodeProgress = std::max(0.0, std::min(1.0, explodeProgress));

                int count = std::min({ (int)actors.size(),
                                       (int)originalPositions.size(),
                                       (int)explodedPositions.size() });
                for (int i = 0; i < count; i++)
                {
                    if (!actors[i]) continue;
                    actors[i]->SetPosition(
                        originalPositions[i][0] + (explodedPositions[i][0] - originalPositions[i][0]) * explodeProgress,
                        originalPositions[i][1] + (explodedPositions[i][1] - originalPositions[i][1]) * explodeProgress,
                        originalPositions[i][2] + (explodedPositions[i][2] - originalPositions[i][2]) * explodeProgress
                    );
                    actors[i]->Modified();
                }
            }
        }

        // Reset model actors back to their original VR transforms.
        // This is handled inside the VR thread to avoid cross-thread VTK issues.
        if (resetRequested.exchange(false))
        {
            animationEnabled = false;
            explodeActive    = false;
            explodeProgress  = 0.0;

            int count = std::min(
                actors.size(),
                std::min(originalPositions.size(),
                    std::min(originalScales.size(), originalOrientations.size()))
            );

            for (int i = 0; i < count; i++)
            {
                vtkActor* actor = actors[i];

                if (!actor)
                {
                    continue;
                }

                actor->SetPosition(
                    originalPositions[i][0],
                    originalPositions[i][1],
                    originalPositions[i][2]
                );

                actor->SetScale(
                    originalScales[i][0],
                    originalScales[i][1],
                    originalScales[i][2]
                );

                actor->SetOrientation(
                    originalOrientations[i][0],
                    originalOrientations[i][1],
                    originalOrientations[i][2]
                );

                actor->Modified();
            }
        }

        // Simple turntable animation for the loaded CAD model.
        // Only actors in the actors list are animated, so the floor and sky sphere stay still.
        if (animationEnabled)
        {
            for (auto actor : actors)
            {
                if (actor)
                {
                    actor->RotateY(0.6);
                    actor->Modified();
                }
            }
        }

        if (vrRenderWindow)
        {
            vrRenderWindow->Render();
        }

        QThread::msleep(10);
    }

    // Cleanup after the VR loop exits.
    if (vrInteractor)
    {
        vrInteractor->TerminateApp();
    }

    if (vrRenderWindow)
    {
        vrRenderWindow->Finalize();
    }

    vrInteractor = nullptr;
    vrRenderWindow = nullptr;

}