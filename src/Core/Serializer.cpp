#include "Serializer.h"
#include "Utils/FileUtils.h"
#include "Core/Editor.h"
#include "Simulation/Simulation.h"
#include "Panels/PlanePanel.h"

std::string Serializer::SerializeEditor(const Ref<Editor> editor) {
    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "Current Simulation" << YAML::Value << editor->GetSimulation()->Filepath();
    out << YAML::Key << "Selected Resource" << YAML::Value << editor->SelectedID();

    out << YAML::Key << "Camera Settings" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "Type" << YAML::Value << static_cast<int>(editor->GetCamera()->GetProperties().Type);
    out << YAML::Key << "Position" << YAML::Value << editor->GetCamera()->GetProperties().Position;
    out << YAML::Key << "Focal Point" << YAML::Value << editor->GetCamera()->GetProperties().FocalPoint;
    out << YAML::Key << "FOV" << YAML::Value << editor->GetCamera()->GetProperties().FOV;
    out << YAML::Key << "NearClip" << YAML::Value << editor->GetCamera()->GetProperties().NearClip;
    out << YAML::Key << "FarClip" << YAML::Value << editor->GetCamera()->GetProperties().FarClip;
    out << YAML::Key << "OrthographicSize" << YAML::Value << editor->GetCamera()->GetProperties().OrthographicSize;
    out << YAML::Key << "OrthographicNear" << YAML::Value << editor->GetCamera()->GetProperties().OrthographicNear;
    out << YAML::Key << "OrthographicFar" << YAML::Value << editor->GetCamera()->GetProperties().OrthographicFar;
    out << YAML::Key << "Pitch" << YAML::Value << editor->GetCamera()->GetProperties().Pitch;
    out << YAML::Key << "Yaw" << YAML::Value << editor->GetCamera()->GetProperties().Yaw;
    out << YAML::Key << "Distance" << YAML::Value << editor->GetCamera()->GetProperties().Distance;
    out << YAML::EndMap;

    out << YAML::Key << "Panels" << YAML::Value;
    out << YAML::BeginMap;
    std::vector<Ref<Panel>> panels = editor->GetPanels();
    for (auto panel : panels)
        out << YAML::Key << panel->Name() << YAML::Value << panel->m_Enabled;
    out << YAML::EndMap;

	out << YAML::Key << "Panel Settings" << YAML::Value;
	out << YAML::BeginMap;
	for (auto panel : panels) {
		out << YAML::Key << panel->Name() << YAML::Value;
		out << YAML::BeginMap;
		if (panel->Name() == "Coordinate Plane Properties") {
			Ref<PlanePanel> pp = std::dynamic_pointer_cast<PlanePanel>(panel);
			PlanePanelSettings settings = pp->Settings();
			out << YAML::Key << "ShowGrid" << YAML::Value << settings.ShowGrid;
			out << YAML::Key << "Mirror" << YAML::Value << settings.Mirror;
			out << YAML::Key << "XAxis" << YAML::Value << settings.XAxis;
			out << YAML::Key << "YAxis" << YAML::Value << settings.YAxis;
			out << YAML::Key << "ZAxis" << YAML::Value << settings.ZAxis;
			out << YAML::Key << "Length" << YAML::Value << settings.Length;
			out << YAML::Key << "StepSize" << YAML::Value << settings.StepSize;
		}
		out << YAML::EndMap;
	}
	out << YAML::EndMap;

    out << YAML::EndMap;
    return std::string(out.c_str());
}

std::string Serializer::SerializeSimulation(const Ref<Simulation> simulation) {
    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "Sources" << YAML::Value;
    out << YAML::BeginSeq;
    for (Ref<Source> source : simulation->Sources()) {
        out << YAML::BeginMap;
        out << YAML::Key << "ID" << YAML::Value << source->ID();
        out << YAML::Key << "Name" << YAML::Value << source->Name();
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    out << YAML::Key << "Sinks" << YAML::Value;
    out << YAML::BeginSeq;
    for (Ref<Sink> sink : simulation->Sinks()) {
        out << YAML::BeginMap;
        out << YAML::Key << "ID" << YAML::Value << sink->ID();
        out << YAML::Key << "Name" << YAML::Value << sink->Name();
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    out << YAML::Key << "Grids" << YAML::Value;
    out << YAML::BeginSeq;
    for (Ref<Grid> grid : simulation->Grids()) {
        out << YAML::BeginMap;
        out << YAML::Key << "ID" << YAML::Value << grid->ID();
        out << YAML::Key << "Name" << YAML::Value << grid->Name();
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    out << YAML::Key << "Particles" << YAML::Value;
    out << YAML::BeginSeq;
    for (Ref<Particle> particle : simulation->Particles()) {
        out << YAML::BeginMap;
        out << YAML::Key << "ID" << YAML::Value << particle->ID();
        out << YAML::Key << "Name" << YAML::Value << particle->Name();
        out << YAML::Key << "Position" << YAML::Value << particle->Position();
        out << YAML::Key << "Velocity" << YAML::Value << particle->Velocity();
        out << YAML::Key << "Mass" << YAML::Value << particle->Mass();
        out << YAML::Key << "Radius" << YAML::Value << particle->Radius();
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

	out << YAML::Key << "Length Unit" << YAML::Value << (int)simulation->LengthUnit();
	out << YAML::Key << "Simulation Length" << YAML::Value << simulation->Length();
	out << YAML::Key << "Safeguard Cache Enabled" << YAML::Value << simulation->SafeguardCacheEnabled();
	out << YAML::Key << "Simulation Record Enabled" << YAML::Value << simulation->SimulationRecordEnabled();
	out << YAML::Key << "Solver" << YAML::Value << (int)simulation->Solver();
	out << YAML::Key << "Bounds" << YAML::Value << simulation->Bounds();
	out << YAML::Key << "Dynamic Timestep" << YAML::Value << simulation->DynamicTimestep();
	out << YAML::Key << "Timestep" << YAML::Value << simulation->Timestep();
	out << YAML::Key << "Local Workers" << YAML::Value << simulation->NumLocalWorkers();
	out << YAML::Key << "Remote Workers" << YAML::Value << simulation->NumRemoteWorkers();
	out << YAML::Key << "Unit Size" << YAML::Value << simulation->UnitSize();

    out << YAML::EndMap;
    return std::string(out.c_str());
}

bool Serializer::DeserializeEditor(Ref<Editor> editor, const std::string& data) {
    YAML::Node yamldata = YAML::Load(data);

    // General editor settings
    if (yamldata["Selected Resource"]) {
        editor->SetSelectedID(yamldata["Selected Resource"].as<uint64_t>());
    } else WARN("No selected resource detected, defaulting to none");

    // Simulation deserialization
    if (yamldata["Current Simulation"] && yamldata["Current Simulation"].as<std::string>() != "") {
        if (!DeserializeSimulation(editor->GetSimulation(), yamldata["Current Simulation"].as<std::string>()))
            WARN("Unable to properly deserialize simulation!");
    } else WARN("No prior simulation to load detected, defaulting to a new simulation");

    // Camera deserialization
    if (yamldata["Camera Settings"]) {
        CameraProperties cameraprops = editor->GetCamera()->GetProperties();
        if (yamldata["Camera Settings"]["Type"])
            cameraprops.Type = static_cast<CameraTypes>(yamldata["Camera Settings"]["Type"].as<int>());
        else WARN("Camera type not found in editor settings!");
        if (yamldata["Camera Settings"]["Position"])
            cameraprops.Position = yamldata["Camera Settings"]["Position"].as<glm::vec3>();
        else WARN("Camera position not found in editor settings!");
        if (yamldata["Camera Settings"]["Focal Point"])
            cameraprops.FocalPoint = yamldata["Camera Settings"]["Focal Point"].as<glm::vec3>();
        else WARN("Camera focal point not found in editor settings!");
        if (yamldata["Camera Settings"]["FOV"])
            cameraprops.FOV = yamldata["Camera Settings"]["FOV"].as<float>();
        else WARN("Camera FOV not found in editor settings!");
        if (yamldata["Camera Settings"]["NearClip"])
            cameraprops.NearClip = yamldata["Camera Settings"]["NearClip"].as<float>();
        else WARN("Camera NearClip not found in editor settings!");
        if (yamldata["Camera Settings"]["FarClip"])
            cameraprops.FarClip = yamldata["Camera Settings"]["FarClip"].as<float>();
        else WARN("Camera FarClip not found in editor settings!");
        if (yamldata["Camera Settings"]["OrthographicSize"])
            cameraprops.OrthographicSize = yamldata["Camera Settings"]["OrthographicSize"].as<float>();
        else WARN("Camera OrthographicSize not found in editor settings!");
        if (yamldata["Camera Settings"]["OrthographicNear"])
            cameraprops.OrthographicNear = yamldata["Camera Settings"]["OrthographicNear"].as<float>();
        else WARN("Camera OrthographicNear not found in editor settings!");
        if (yamldata["Camera Settings"]["OrthographicFar"])
            cameraprops.OrthographicFar = yamldata["Camera Settings"]["OrthographicFar"].as<float>();
        else WARN("Camera OrthographicFar not found in editor settings!");
        if (yamldata["Camera Settings"]["Pitch"])
            cameraprops.Pitch = yamldata["Camera Settings"]["Pitch"].as<float>();
        else WARN("Camera Pitch not found in editor settings!");
        if (yamldata["Camera Settings"]["Yaw"])
            cameraprops.Yaw = yamldata["Camera Settings"]["Yaw"].as<float>();
        else WARN("Camera Yaw not found in editor settings!");
        if (yamldata["Camera Settings"]["Distance"])
            cameraprops.Distance = yamldata["Camera Settings"]["Distance"].as<float>();
        else WARN("Camera Distance not found in editor settings!");
        editor->GetCamera()->SetProperties(cameraprops);
    } else WARN("Camera properties not found in editor settings!");

    // Panel deserialization
    if (yamldata["Panels"]) {
        std::vector<Ref<Panel>> panels = editor->GetPanels();
        for (auto panel : panels) {
            if (yamldata["Panels"][panel->Name()]) {
                panel->m_Enabled =yamldata["Panels"][panel->Name()].as<bool>();
            } else WARN("\"{}\" panel setting not detected!", panel->Name());
        }
    } else WARN("panel settings not found in editor settings!");

	// Custom panel deserialization
	if (yamldata["Panel Settings"]) {
		std::vector<Ref<Panel>> panels = editor->GetPanels();
		for (auto panel : panels) {
			if (panel->Name() == "Coordinate Plane Properties") {
				if (yamldata["Panel Settings"][panel->Name()]) {
					Ref<PlanePanel> pp = std::dynamic_pointer_cast<PlanePanel>(panel);
					PlanePanelSettings& settings = pp->Settings();
					if (yamldata["Panel Settings"][panel->Name()]["ShowGrid"]) {
                        settings.ShowGrid = yamldata["Panel Settings"][panel->Name()]["ShowGrid"].as<bool>();
                    } else WARN("Missing setting detected for the coodinate plane properties panel");
					if (yamldata["Panel Settings"][panel->Name()]["Mirror"]) {
                        settings.Mirror = yamldata["Panel Settings"][panel->Name()]["Mirror"].as<bool>();
                    } else WARN("Missing setting detected for the coodinate plane properties panel");
					if (yamldata["Panel Settings"][panel->Name()]["XAxis"]) {
                        settings.XAxis = yamldata["Panel Settings"][panel->Name()]["XAxis"].as<bool>();
                    } else WARN("Missing setting detected for the coodinate plane properties panel");
					if (yamldata["Panel Settings"][panel->Name()]["YAxis"]) {
                        settings.YAxis = yamldata["Panel Settings"][panel->Name()]["YAxis"].as<bool>();
                    } else WARN("Missing setting detected for the coodinate plane properties panel");
					if (yamldata["Panel Settings"][panel->Name()]["ZAxis"]) {
                        settings.ZAxis = yamldata["Panel Settings"][panel->Name()]["ZAxis"].as<bool>();
                    } else WARN("Missing setting detected for the coodinate plane properties panel");
					if (yamldata["Panel Settings"][panel->Name()]["Length"]) {
                        settings.Length = yamldata["Panel Settings"][panel->Name()]["Length"].as<float>();
                    } else WARN("Missing setting detected for the coodinate plane properties panel");
					if (yamldata["Panel Settings"][panel->Name()]["StepSize"]) {
                        settings.StepSize = yamldata["Panel Settings"][panel->Name()]["StepSize"].as<float>();
                    } else WARN("Missing setting detected for the coodinate plane properties panel");
				} else WARN("Custom settings for the coordinate plane properties panel not found!");
			}
		}
	} else WARN("Custom panel settings not found in editor settings!");

    return true;
}

bool Serializer::DeserializeSimulation(Ref<Simulation> simulation, const std::string& filepath) {
    YAML::Node yamldata = YAML::Load(FileUtils::Read(filepath));
    simulation->SetFilepath(filepath);

    if (yamldata["Sources"]) {
        auto sources = yamldata["Sources"];
        for (auto sourcedata : sources) {
            if (sourcedata["ID"] && 
                sourcedata["Name"]) {
                Ref<Source> source = CreateRef<Source>(sourcedata["ID"].as<uint64_t>());
                source->SetName(sourcedata["Name"].as<std::string>());
                simulation->Sources().push_back(source);
            } else WARN("Resource data for serialized source is missing critical data - deserialization for this resource will be skipped");
        }
    } else WARN("No sources found to serialize into simulation!");

    if (yamldata["Sinks"]) {
        auto sinks = yamldata["Sinks"];
        for (auto sinkdata : sinks) {
            if (sinkdata["ID"] && 
                sinkdata["Name"]) {
                Ref<Sink> sink = CreateRef<Sink>(sinkdata["ID"].as<uint64_t>());
                sink->SetName(sinkdata["Name"].as<std::string>());
                simulation->Sinks().push_back(sink);
            } else WARN("Resource data for serialized sink is missing critical data - deserialization for this resource will be skipped");
        }
    } else WARN("No sinks found to serialize into simulation!");

    if (yamldata["Grids"]) {
        auto grids = yamldata["Grids"];
        for (auto griddata : grids) {
            if (griddata["ID"] && 
                griddata["Name"]) {
                Ref<Grid> grid = CreateRef<Grid>(griddata["ID"].as<uint64_t>());
                grid->SetName(griddata["Name"].as<std::string>());
                simulation->Grids().push_back(grid);
            } else WARN("Resource data for serialized source is missing critical data - deserialization for this resource will be skipped");
        }
    } else WARN("No sources found to serialize into simulation!");

    if (yamldata["Particles"]) {
        auto particles = yamldata["Particles"];
        for (auto particledata : particles) {
            if (particledata["ID"] && 
                particledata["Name"] &&
                particledata["Position"] &&
                particledata["Velocity"] &&
                particledata["Radius"] &&
                particledata["Mass"]) {
                Ref<Particle> particle = CreateRef<Particle>(particledata["ID"].as<uint64_t>());
                particle->SetName(particledata["Name"].as<std::string>());
                particle->SetPosition(particledata["Position"].as<glm::vec3>());
                particle->SetVelocity(particledata["Velocity"].as<glm::vec3>());
                particle->SetRadius(particledata["Radius"].as<float>());
                particle->SetMass(particledata["Mass"].as<float>());
                simulation->Particles().push_back(particle);
            } else WARN("Resource data for serialized source is missing critical data - deserialization for this resource will be skipped");
        }
    } else WARN("No sources found to serialize into simulation!");

	if (yamldata["Length Unit"]) {
		simulation->SetLengthUnit((SimulationLengthUnit)yamldata["Length Unit"].as<int>());
	} else WARN("No length unit found to serialize into simulation!");

	if (yamldata["Simulation Length"]) {
		simulation->SetLength(yamldata["Simulation Length"].as<uint64_t>());
	} else WARN("No dedicated simulation length found to serialize into simulation!");

	if (yamldata["Safeguard Cache Enabled"]) {
		simulation->SetSafeguardCache(yamldata["Safeguard Cache Enabled"].as<bool>());
	} else WARN("No safeguard cache setting found to serialize into simulation!");

	if (yamldata["Simulation Record Enabled"]) {
		simulation->SetSimulationRecord(yamldata["Simulation Record Enabled"].as<bool>());
	} else WARN("No simulation record setting found to serialize into simulation!");

	if (yamldata["Solver"]) {
		simulation->SetSolver((SimulationSolver)yamldata["Solver"].as<int>());
	} else WARN("No solver found to serialize into simulation!");

	if (yamldata["Bounds"]) {
		simulation->SetBounds(yamldata["Bounds"].as<glm::vec2>());
	} else WARN("No bounds found to serialize into simulation!");

	if (yamldata["Dynamic Timestep"]) {
		simulation->SetDynamicTimestep(yamldata["Dynamic Timestep"].as<bool>());
	} else WARN("No dynamic timestep found to serialize into simulation!");

	if (yamldata["Timestep"]) {
		simulation->SetTimestep(yamldata["Timestep"].as<uint64_t>());
	} else WARN("No timestep found to serialize into simulation!");

	if (yamldata["Local Workers"]) {
		simulation->SetNumLocalWorkers(yamldata["Local Workers"].as<uint32_t>());
	} else WARN("No local workers found to serialize into simulation!");

	if (yamldata["Remote Workers"]) {
		simulation->SetNumRemoteWorkers(yamldata["Remote Workers"].as<uint32_t>());
	} else WARN("No remote workers found to serialize into simulation!");

	if (yamldata["Unit Size"]) {
		simulation->SetUnitSize(yamldata["Unit Size"].as<double>());
	} else WARN("No unit size found to serialize into simulation!");

    return true;
}

bool Serializer::SimulationSaved(const Ref<Simulation> simulation) {
    if (simulation->Filepath() == "") return false;
    std::string saved_content = FileUtils::Read(simulation->Filepath());
    if (saved_content == SerializeSimulation(simulation)) return true;
    return false;
}
