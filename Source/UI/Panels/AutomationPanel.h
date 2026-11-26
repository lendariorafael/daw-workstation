#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"

// Editor de curvas: os pontos ficam guardados no projeto Opalo.
class AutomationPanel : public juce::Component
{
public:
    explicit AutomationPanel(ProjectModel& m) : model(m)
    {
        addAndMakeVisible(lanes);
        lanes.addItem("Master volume", 1);
        lanes.addItem("Track 1 volume", 2);
        lanes.addItem("Track 1 pan", 3);
        lanes.setSelectedId(1);
        lanes.onChange = [this] { repaint(); };
        addAndMakeVisible(addPoint);
        addPoint.setButtonText("+ Point");
        addPoint.onClick = [this] { addAutomationPoint(); };
        addAndMakeVisible(newLane);
        newLane.setButtonText("+ Lane");
        newLane.onClick = [this]
        {
            AutomationLane lane;
            lane.name = "Automation " + juce::String(model.automationLanes.size() + 1);
            lane.target = "Master volume";
            lane.points = { {0.0, 0.8f}, {4.0, 0.8f} };
            model.automationLanes.push_back(std::move(lane));
            model.changed();
            repaint();
        };
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);
        auto graph = getLocalBounds().reduced(14).withTrimmedTop(42);
        g.setColour(DawLookAndFeel::panelLight);
        g.fillRoundedRectangle(graph.toFloat(), 5.0f);
        for (int i=0;i<=8;++i)
        {
            const float x=(float)graph.getX()+graph.getWidth()*i/8.0f;
            g.setColour(DawLookAndFeel::muted.withAlpha(0.22f));
            g.drawVerticalLine((int)x,(float)graph.getY(),(float)graph.getBottom());
        }
        for (int i=0;i<=4;++i)
        {
            const float y=(float)graph.getY()+graph.getHeight()*i/4.0f;
            g.drawHorizontalLine((int)y,(float)graph.getX(),(float)graph.getRight());
        }
        if (model.automationLanes.empty()) return;
        const auto& lane=model.automationLanes.back();
        juce::Path path;
        bool first=true;
        for (const auto& p:lane.points)
        {
            const float x=(float)graph.getX()+(float)(p.time/8.0)*graph.getWidth();
            const float y=(float)graph.getBottom()-p.value*graph.getHeight();
            if(first){path.startNewSubPath(x,y);first=false;} else path.lineTo(x,y);
            g.setColour(lane.colour); g.fillEllipse(x-4,y-4,8,8);
        }
        g.setColour(lane.colour); g.strokePath(path,juce::PathStrokeType(2.0f));
        g.setColour(DawLookAndFeel::text);
        g.setFont(DawLookAndFeel::uiFont(11.0f));
        g.drawText(lane.name+"  ·  "+lane.target,graph.removeFromTop(18),juce::Justification::centredLeft);
    }

    void resized() override
    {
        auto r=getLocalBounds().reduced(10);
        auto top=r.removeFromTop(28);
        lanes.setBounds(top.removeFromLeft(210));
        newLane.setBounds(top.removeFromLeft(78).reduced(3,0));
        addPoint.setBounds(top.removeFromLeft(92).reduced(3,0));
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if(model.automationLanes.empty()) return;
        auto graph=getLocalBounds().reduced(14).withTrimmedTop(42);
        if(!graph.contains(e.getPosition())) return;
        const double time=juce::jlimit(0.0,8.0,(e.x-graph.getX())*8.0/juce::jmax(1,graph.getWidth()));
        const float value=juce::jlimit(0.0f,1.0f,1.0f-(e.y-graph.getY())/(float)juce::jmax(1,graph.getHeight()));
        auto& points=model.automationLanes.back().points;
        points.push_back({time,value});
        std::sort(points.begin(),points.end(),[](const auto&a,const auto&b){return a.time<b.time;});
        model.changed(); repaint();
    }

private:
    void addAutomationPoint()
    {
        if(model.automationLanes.empty())
        {
            AutomationLane lane; lane.name="Master Volume"; lane.target="Master volume";
            lane.points={{0.0,0.8f},{8.0,0.8f}};
            model.automationLanes.push_back(std::move(lane));
        }
        else
        {
            auto& p=model.automationLanes.back().points;
            p.push_back({p.empty()?0.0:p.back().time+1.0,p.empty()?0.8f:p.back().value});
        }
        model.changed(); repaint();
    }
    ProjectModel& model;
    juce::ComboBox lanes;
    juce::TextButton newLane, addPoint;
};
