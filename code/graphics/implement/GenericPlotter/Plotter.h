//# Plotter.h: Highest level plotting object that holds one or more canvases.
//# Copyright (C) 2008
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#ifndef PLOTTER_H_
#define PLOTTER_H_

#include <graphics/GenericPlotter/PlotCanvasLayout.h>
#include <graphics/GenericPlotter/PlotLogger.h>
#include <graphics/GenericPlotter/PlotPanel.h>

namespace casa {

//# Forward Declarations
class PlotFactory;


// A Plotter can be thought of as a frame that holds one or more PlotCanvases
// in a configuration determined by a given PlotCanvasLayout.  It also has
// some top-level and main-window functionality.
class Plotter {
public:
    // This enum should contain all known implementations.
    enum Implementation {
        QWT,
        
        // Default plotter implementation is set here.
        DEFAULT = QWT
    };
    
    // Default implementation-specific GUI panels that can be turned on or off.
    enum DefaultPanel {
        HAND_TOOLS,  // buttons/tools to: select regions, zoom, and pan;
                     // (optionally) turn on/off tracker;
                     // (optionally) show/hide and position legend
        EXPORT_TOOLS // buttons/tools to export the canvases to a file
    };
    
    
    // Constructor.
    Plotter();
    
    // Destructor.
    virtual ~Plotter();
    
    
    // ABSTRACT METHODS //
    
    // Top-Level GUI methods //
    
    // Shows/hides the plotter GUI.
    virtual void showGUI(bool showGUI = true) = 0;
    
    // Returns the current size of the plotter GUI in pixels (width x height).
    virtual pair<int, int> size() const = 0;
    
    // Sets the plotter GUI size in pixels
    virtual void setSize(int width, int height) = 0;
    
    // Returns the plotter window's title.
    virtual String windowTitle() const = 0;
    
    // Sets the plotter window's title to the given.
    virtual void setWindowTitle(const String& newTitle) = 0;
    
    // Returns the size of the canvas area (i.e., minus bordering panels) in
    // pixels (width x height).
    virtual pair<int, int> canvasAreaSize() const = 0;
    
    // If the plotter has a single canvas, sets the size of the canvas to the
    // given.  If resizeWindow is true, the plotter is resized to fit the new
    // canvas size.
    virtual void setCanvasSize(int width, int height,
                               bool resizeWindow = true) = 0;
    
    // Returns the DPI used in the GUI display.
    virtual int displayDPI() const = 0;
    
    // Returns true if the plotter window can be casted to a QWidget, false
    // otherwise.
    virtual bool isQWidget() const = 0;
    
    // Gets/sets cursor for the whole plotter.  Can be overridden by individual
    // canvases.
    // <group>
    virtual PlotCursor cursor() const = 0;
    virtual void setCursor(PlotCursor cursor) = 0;
    // </group>
    
    // Refreshes the plotter GUI.
    virtual void refresh() = 0;
    
    // Closes the plotter window.
    virtual void close() = 0;
   
    
    // Canvas Layout methods //
    
    // Returns the current layout, or a null pointer if none has been set.
    virtual PlotCanvasLayoutPtr canvasLayout() = 0;
    
    // Sets the canvas layout to the given.  Clears out old canvases as needed.
    virtual void setCanvasLayout(PlotCanvasLayoutPtr layout) = 0;
    
    // Method for when the layout has changed (i.e. changed canvases, etc.).
    // This should only be used by the layout currently being used by the
    // plotter.
    virtual void canvasLayoutChanged(PlotCanvasLayout& layout) = 0;
   
    
    // Panel methods //
    
    // Returns whether or not the given default panel is shown.
    virtual bool defaultPanelShown(DefaultPanel panel) = 0;
    
    // Shows/hides the given default panel.  Note: the default panels are for
    // convenience's sake and are left completely up to the implementation.
    virtual void showDefaultPanel(DefaultPanel panel, bool show = true) = 0;
    
    // Adds the given plot panel and returns its index.
    virtual int addPanel(PlotPanelPtr panel) = 0;
    
    // Returns all plot panels currently shown.
    virtual vector<PlotPanelPtr> allPanels() = 0;
    
    // Returns the number of plot panels currently on the plotter.
    virtual unsigned int numPanels() = 0;
    
    // Returns the plot panel at the given index, or a null pointer for an
    // invalid index.
    virtual PlotPanelPtr getPanel(int index) = 0;
    
    // Returns the index of the given plot panel, or -1 if not on canvas or
    // null.
    virtual int panelIndex(PlotPanelPtr panel) = 0;
    
    // Clears all plot panels from the plotter.
    virtual void clearPanels() = 0;
    
    // Removes the given plot panel from the plotter.
    virtual void removePanel(PlotPanelPtr panel) = 0;
    
    // Removes the plot panel with the given index from the plotter.
    virtual void removePanel(int id) = 0;
    
    // Removes the last-added plot panel from the plotter.
    virtual void removeLastPanel() = 0;
    
    
    // Plotting Functionality methods //
    
    // Returns the implementation of this plotter.
    virtual Implementation implementation() const = 0;
    
    // Returns a new instance of a PlotFactory that can create plot items for
    // this implementation.  It is the caller's responsibility to delete the
    // PlotFactory when finished.
    virtual PlotFactory* implementationFactory() const = 0;
    
    // Exports the plotter (all canvases) using the given format.
    virtual bool exportToFile(const PlotExportFormat& format) = 0;
    
    // Shows a file chooser dialog and returns the absolute filename that the
    // user chooses.  If a directory is given, start the dialog there.  If the
    // user cancels, an empty String is returned.
    virtual String fileChooserDialog(const String& title = "File Chooser",
                                     const String& directory = "") = 0;
    
    
    // Logging methods //
    
    // Gets/sets which events, as a flag, this Plotter should log.  The flag
    // should be a bitwise-or of one or more values in PlotLogger::Event.  This
    // call should trickle down to relevant objects associated with this
    // plotter as necessary.
    // <group>
    virtual int logEventFlags() const = 0;
    virtual void setLogEventFlags(int flags) = 0;
    // </group>
    
    
    // Event Handling methods //
    
    // Registers the given resize event handler with this plotter.
    virtual void registerResizeHandler(PlotResizeEventHandlerPtr handler) = 0;
    
    // Returns a list of all registered resize event handlers for this plotter.
    virtual vector<PlotResizeEventHandlerPtr> allResizeHandlers() const = 0;
    
    // Unregisters the given resize event handler with this plotter.
    virtual void unregisterResizeHandler(PlotResizeEventHandlerPtr handler) =0;
    
    
    // IMPLEMENTED METHODS //
    
    // See PlotCanvasLayout::canvasAt().
    virtual PlotCanvasPtr canvasAt(const PlotLayoutCoordinate& coord);
    
    // See PlotCanvasLayout::canvas().
    virtual PlotCanvasPtr canvas();
    
    // Sets the layout to a single layout with the given canvas.
    virtual void setCanvas(PlotCanvasPtr canvas);
    
    // Gets the PlotLogger associated with this Plotter.  There should be
    // exactly one PlotLogger per Plotter.
    virtual PlotLoggerPtr logger();
    
protected:
    // Logger.
    PlotLoggerPtr m_logger;
};
typedef CountedPtr<Plotter> PlotterPtr;

}

#endif /*PLOTTER_H_*/
