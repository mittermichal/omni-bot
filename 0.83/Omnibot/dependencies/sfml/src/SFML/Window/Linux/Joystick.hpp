////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef SFML_JOYSTICKLINUX_HPP
#define SFML_JOYSTICKLINUX_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// Linux implementation of Joystick
////////////////////////////////////////////////////////////
class Joystick
{
public :

    ////////////////////////////////////////////////////////////
    /// Initialize the instance and bind it to a physical joystick
    ///
    /// \param Index : Index of the physical joystick to bind to
    ///
    ////////////////////////////////////////////////////////////
    void Initialize(unsigned int Index);

    ////////////////////////////////////////////////////////////
    /// Update the current joystick and return its new state
    ///
    /// \return Current state of the joystick
    ///
    ////////////////////////////////////////////////////////////
    JoystickState UpdateState();

    ////////////////////////////////////////////////////////////
    /// Get the number of axes supported by the joystick
    ///
    /// \return Number of axis
    ///
    ////////////////////////////////////////////////////////////
    unsigned int GetAxesCount() const;

    ////////////////////////////////////////////////////////////
    /// Get the number of buttons supported by the joystick
    ///
    /// \return Number of buttons
    ///
    ////////////////////////////////////////////////////////////
    unsigned int GetButtonsCount() const;

private :

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    int           myDescriptor; ///< Linux descriptor of the joystick
    unsigned int  myNbAxes;     ///< Number of axis supported by the joystick
    unsigned int  myNbButtons;  ///< Number of buttons supported by the joystick
    JoystickState myState;      ///< Current state of the joystick
};

} // namespace priv

} // namespace sf


#endif // SFML_JOYSTICKLINUX_HPP