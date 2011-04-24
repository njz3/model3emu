#include "Supermodel.h"

// All the input subclasses have been grouped together here as they are very simple classes

/*
 * CSwitchInput
 */
CSwitchInput::CSwitchInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, UINT16 offVal, UINT16 onVal) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_SWITCH, inputGameFlags, defaultMapping), m_offVal(offVal), m_onVal(onVal)
{
	//
}

void CSwitchInput::Poll()
{
	prevValue = value;

	bool boolValue = !!value;
	if (m_source != NULL && m_source->GetValueAsSwitch(boolValue))
		value = (boolValue ? m_onVal : m_offVal);
	else
		value = m_offVal;
}

bool CSwitchInput::Pressed()
{
	return prevValue == m_offVal && value == m_onVal;
}

bool CSwitchInput::Released()
{
	return prevValue == m_onVal && value == m_offVal;
}

/*
 * CAnalogInput
 */
CAnalogInput::CAnalogInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, UINT16 minVal, UINT16 maxVal) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_ANALOG, inputGameFlags, defaultMapping, minVal), m_minVal(minVal), m_maxVal(maxVal) 
{
	//
}

void CAnalogInput::Poll()
{
	prevValue = value;

	if (m_source == NULL)
	{
		value = m_minVal;
		return;
	}
	int intValue = value;
	if (m_source->GetValueAsAnalog(intValue, m_minVal, m_minVal, m_maxVal))
		value = intValue;
	else
		value = m_minVal;
}

bool CAnalogInput::HasValue()
{
	return value > m_minVal;
}

double CAnalogInput::ValueAsFraction()
{
	double frac = (double)(value - m_minVal)/(double)(m_maxVal - m_minVal);
	return (frac >= 0.0 ? frac : -frac);
}

/*
 * CAxisInput
 */
CAxisInput::CAxisInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, 
	CAnalogInput *negInput, CAnalogInput *posInput,	UINT16 minVal, UINT16 offVal, UINT16 maxVal) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_AXIS, inputGameFlags, defaultMapping, offVal), m_negInput(negInput), m_posInput(posInput), 
		m_minVal(minVal), m_offVal(offVal), m_maxVal(maxVal)
{
	//
}

void CAxisInput::Poll()
{
	prevValue = value;

	// Try getting value from analog inputs that represent negative and positive range of the axis first and then try the default input source
	int intValue = value;
	if ((m_negInput != NULL && m_negInput->HasValue()) || (m_posInput != NULL && m_posInput->HasValue()))
	{
		if (m_maxVal > m_minVal)
		{
			value = m_offVal;
			if (m_posInput != NULL) value += (int)(m_posInput->ValueAsFraction() * (double)(m_maxVal - m_offVal));
			if (m_negInput != NULL) value -= (int)(m_negInput->ValueAsFraction() * (double)(m_offVal - m_minVal));
		}
		else
		{ 
			value = m_offVal;
			if (m_posInput != NULL) value += (int)(m_posInput->ValueAsFraction() * (double)(m_offVal - m_maxVal));
			if (m_negInput != NULL) value -= (int)(m_negInput->ValueAsFraction() * (double)(m_minVal - m_offVal));
		}
	}
	else if (m_source != NULL && m_source->GetValueAsAnalog(intValue, m_minVal, m_offVal, m_maxVal))
		value = intValue;
	else 
		value = m_offVal;
}

bool CAxisInput::HasValue()
{
	return value != m_offVal;
}

double CAxisInput::ValueAsFraction()
{
	double frac = (double)(value - m_minVal)/(double)(m_maxVal - m_minVal);
	return (frac >= 0.0 ? frac : -frac);
}

/*
 * CGearShift4Input
 */
CGearShift4Input::CGearShift4Input(const char *inputId, const char *inputLabel, unsigned inputGameFlags,
		CSwitchInput *shift1Input, CSwitchInput *shift2Input, CSwitchInput *shift3Input, CSwitchInput *shift4Input,
		CSwitchInput *shiftUpInput, CSwitchInput *shiftDownInput) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_VIRTUAL, inputGameFlags),
		m_shift1Input(shift1Input), m_shift2Input(shift2Input), m_shift3Input(shift3Input), m_shift4Input(shift4Input),
		m_shiftUpInput(shiftUpInput), m_shiftDownInput(shiftDownInput)
{
	//
}

void CGearShift4Input::Poll()
{
	prevValue = value;

	// Neutral is when all gear buttons are released so shifting here is implemented as follows:
	// Gears (values 1-4) are set by pressing a button (lower gears have priority) and "stick" until a shift to another gear or until the
	// button is pressed again, at which point neutral (value 0) is assumed.
	if      (m_shift1Input->Pressed()) value = (value == 1 ? 0 : 1);
	else if (m_shift2Input->Pressed()) value = (value == 2 ? 0 : 2);
	else if (m_shift3Input->Pressed()) value = (value == 3 ? 0 : 3);
	else if (m_shift4Input->Pressed()) value = (value == 4 ? 0 : 4);

	// Also the shift up/down controls can increase/decrease the gears too
	if      (m_shiftUpInput->Pressed())   value = CInputSource::Clamp(value + 1, 0, 4);
	else if (m_shiftDownInput->Pressed()) value = CInputSource::Clamp(value - 1, 0, 4);
}