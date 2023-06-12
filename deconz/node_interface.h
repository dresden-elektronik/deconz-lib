#ifndef DECONZ_NODE_INTERFACE_H
#define DECONZ_NODE_INTERFACE_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

// forward declarations
class QDialog;
class QWidget;

namespace deCONZ
{

class Node;

/*!
    \ingroup aps
    \class NodeInterface
    \brief Main interface to create a deCONZ plugin.
 */
class NodeInterface
{
public:
    /*! Features the plugin _might_ support. */
    enum Features
    {
        WidgetFeature,
        DialogFeature,
        HttpClientHandlerFeature
    };

    /*! Deconstructor. */
    virtual ~NodeInterface() { }
    /*! Plugin name. */
    virtual const char *name() = 0;
    /*! Returns true if the plugin supports the \p feature. */
    virtual bool hasFeature(Features feature) = 0;
    /*! Returns a widget if the plugin supports the WidgetFeature otherwise 0. */
    virtual QWidget *createWidget() { return 0; }
    /*! Returns a dialog if the plugin supports the DialogFeature otherwise 0. */
    virtual QDialog *createDialog() { return 0; }
};

} // namespace deCONZ

Q_DECLARE_INTERFACE(deCONZ::NodeInterface, "com.deCONZ.NodeInterface/1.0")

#endif // DECONZ_NODE_INTERFACE_H
