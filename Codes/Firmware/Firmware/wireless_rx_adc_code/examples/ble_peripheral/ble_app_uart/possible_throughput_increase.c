
typedef enum 
{
    RX_STANDBY,
    RX_TRANSMISSION
}RX_STATE;

static RX_STATE state = RX_STANDBY;
uint32_t msgIndex, numMsgs;

switch (state)
{
    case RX_STANDBY: // Waiting for the first message chunk
    {
        if (p_data[0] == '?') // first symbol in the first message, marks the start of the sequence
        {
            state = RX_TRANSMISSION;
            msgIndex = 0;
            numMsgs = (uint32_t)p_data[1];
            //serString = sprintf...
            strncopy(serString + strlen(serString), p_data+3, length-3); // first three symbols are part of the header
        }
    }
    case RX_TRANSMISSION: // Collecting further message chunks
    {
        if (p_data[1] == ':') // marks the current message index
        {
            if (p_data[0] == msgIndex)
            {
                //sprintf...;
                strncopy(serString + strlen(serString), p_data+2, length-2); // ignore 2 symbols in the header
                msgIndex++;
            } // ?499,...,499,0:499
            else
            {
                // invalid message arrived (not the one we expected)
                break; // ??
            }
        }
        else if (p_data[0] == '!') // last symbol in the last chunk, marks the end of the sequence
        {
            // send the string to the UART, reset the state
            while(app_uart_put(...) != NRF_SUCCESS);
            state = RX_STANDBY;
        }
    }
}