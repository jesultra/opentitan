// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

use anyhow::{bail, Result};

use std::borrow::Borrow;
use std::time::Duration;

use super::protocol::{ GetCapabilitiesReq, GpioReq, UartReq, SpiReq, I2cReq,
    EmuRequest, EmuResponse, GpioRequest, GpioResponse, I2cRequest, I2cResponse,
    I2cTransferRequest, I2cTransferResponse, Message, ProxyRequest, ProxyResponse, Request,
    Response, SpiRequest, SpiResponse, SpiTransferRequest, SpiTransferResponse, UartRequest,
    UartResponse,
};
use super::CommandHandler;
use crate::app::TransportWrapper;
use crate::bootstrap::Bootstrap;
use crate::io::i2c;
use crate::io::spi;
use crate::transport::TransportError;

/// Implementation of the handling of each protocol request, by means of an underlying
/// `Transport` implementation.
pub struct TransportCommandHandler<'a> {
    transport: &'a TransportWrapper,
}

#[typetag::serde]
impl Request for GetCapabilitiesReq {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError> {
        Ok(Response::GetCapabilities(transport.capabilities()?))
    }
}

#[typetag::serde]
impl Request for GpioReq {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError> {
        let instance = transport.gpio_pin(&self.id)?;
        match self.command {
            GpioRequest::Read => {
                let value = instance.read()?;
                Ok(Response::Gpio(GpioResponse::Read { value }))
            }
            GpioRequest::Write { logic } => {
                instance.write(logic)?;
                Ok(Response::Gpio(GpioResponse::Write))
            }
            GpioRequest::SetMode { mode } => {
                instance.set_mode(mode)?;
                Ok(Response::Gpio(GpioResponse::SetMode))
            }
            GpioRequest::SetPullMode { pull } => {
                instance.set_pull_mode(pull)?;
                Ok(Response::Gpio(GpioResponse::SetPullMode))
            }
        }
    }
}

#[typetag::serde]
impl Request for UartReq {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError> {
        let instance = transport.uart(&self.id)?;
        match &self.command {
            UartRequest::GetBaudrate => {
                let rate = instance.get_baudrate()?;
                Ok(Response::Uart(UartResponse::GetBaudrate { rate }))
            }
            UartRequest::SetBaudrate { rate } => {
                instance.set_baudrate(*rate)?;
                Ok(Response::Uart(UartResponse::SetBaudrate))
            }
            UartRequest::Read {
                timeout_millis,
                len,
            } => {
                let mut data = vec![0u8; *len as usize];
                let count = match timeout_millis {
                    None => instance.read(&mut data)?,
                    Some(ms) => instance
                        .read_timeout(&mut data, Duration::from_millis(*ms as u64))?,
                };
                data.resize(count, 0);
                Ok(Response::Uart(UartResponse::Read { data }))
            }
            UartRequest::Write { data } => {
                instance.write(&data)?;
                Ok(Response::Uart(UartResponse::Write))
            }
        }
    }
}

#[typetag::serde]
impl Request for SpiReq {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError> {
        let instance = transport.spi(&self.id)?;
        match &self.command {
            SpiRequest::GetTransferMode => {
                let mode = instance.get_transfer_mode()?;
                Ok(Response::Spi(SpiResponse::GetTransferMode { mode }))
            }
            SpiRequest::SetTransferMode { mode } => {
                instance.set_transfer_mode(*mode)?;
                Ok(Response::Spi(SpiResponse::SetTransferMode))
            }
            SpiRequest::GetBitsPerWord => {
                let bits_per_word = instance.get_bits_per_word()?;
                Ok(Response::Spi(SpiResponse::GetBitsPerWord { bits_per_word }))
            }
            SpiRequest::SetBitsPerWord { bits_per_word } => {
                instance.set_bits_per_word(*bits_per_word)?;
                Ok(Response::Spi(SpiResponse::SetBitsPerWord))
            }
            SpiRequest::GetMaxSpeed => {
                let speed = instance.get_max_speed()?;
                Ok(Response::Spi(SpiResponse::GetMaxSpeed { speed }))
            }
            SpiRequest::SetMaxSpeed { value } => {
                instance.set_max_speed(*value)?;
                Ok(Response::Spi(SpiResponse::SetMaxSpeed))
            }
            SpiRequest::GetMaxTransferCount => {
                let number = instance.get_max_transfer_count()?;
                Ok(Response::Spi(SpiResponse::GetMaxTransferCount { number }))
            }
            SpiRequest::GetMaxChunkSize => {
                let size = instance.max_chunk_size()?;
                Ok(Response::Spi(SpiResponse::GetMaxChunkSize { size }))
            }
            SpiRequest::SetVoltage { voltage } => {
                instance.set_voltage(*voltage)?;
                Ok(Response::Spi(SpiResponse::SetVoltage))
            }
            SpiRequest::RunTransaction { transaction: reqs } => {
                // Construct proper response to each transfer in request.
                let mut resps: Vec<SpiTransferResponse> = reqs
                    .iter()
                    .map(|transfer| match transfer {
                        SpiTransferRequest::Read { len } => SpiTransferResponse::Read {
                            data: vec![0; *len as usize],
                        },
                        SpiTransferRequest::Write { .. } => SpiTransferResponse::Write,
                        SpiTransferRequest::Both { data } => SpiTransferResponse::Both {
                            data: vec![0; data.len()],
                        },
                    })
                    .collect();
                // Now carefully craft a proper parameter to the
                // `spi::Target::run_transactions()` method.  It will have reference
                // into elements of both the request vector and mutable reference into
                // the response vector.
                let mut transaction: Vec<spi::Transfer> = reqs
                    .iter()
                    .zip(resps.iter_mut())
                    .map(|pair| match pair {
                        (
                            SpiTransferRequest::Read { .. },
                            SpiTransferResponse::Read { data },
                        ) => spi::Transfer::Read(data),
                        (
                            SpiTransferRequest::Write { data },
                            SpiTransferResponse::Write,
                        ) => spi::Transfer::Write(data),
                        (
                            SpiTransferRequest::Both { data: wdata },
                            SpiTransferResponse::Both { data },
                        ) => spi::Transfer::Both(wdata, data),
                        _ => {
                            // This can only happen if the logic in this method is
                            // flawed.  (Never due to network input.)
                            panic!("Mismatch");
                        }
                    })
                    .collect();
                instance.run_transaction(&mut transaction)?;
                Ok(Response::Spi(SpiResponse::RunTransaction {
                    transaction: resps,
                }))
            }
        }
    }
}

#[typetag::serde]
impl Request for I2cReq {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError> {
        let instance = transport.i2c(&self.id)?;
        match &self.command {
            I2cRequest::RunTransaction {
                address,
                transaction: reqs,
            } => {
                // Construct proper response to each transfer in request.
                let mut resps: Vec<I2cTransferResponse> = reqs
                    .iter()
                    .map(|transfer| match transfer {
                        I2cTransferRequest::Read { len } => I2cTransferResponse::Read {
                            data: vec![0; *len as usize],
                        },
                        I2cTransferRequest::Write { .. } => I2cTransferResponse::Write,
                    })
                    .collect();
                // Now carefully craft a proper parameter to the
                // `i2c::Bus::run_transactions()` method.  It will have reference
                // into elements of both the request vector and mutable reference into
                // the response vector.
                let mut transaction: Vec<i2c::Transfer> = reqs
                    .iter()
                    .zip(resps.iter_mut())
                    .map(|pair| match pair {
                        (
                            I2cTransferRequest::Read { .. },
                            I2cTransferResponse::Read { data },
                        ) => i2c::Transfer::Read(data),
                        (
                            I2cTransferRequest::Write { data },
                            I2cTransferResponse::Write,
                        ) => i2c::Transfer::Write(data),
                        _ => {
                            // This can only happen if the logic in this method is
                            // flawed.  (Never due to network input.)
                            panic!("Mismatch");
                        }
                    })
                    .collect();
                instance.run_transaction(*address, &mut transaction)?;
                Ok(Response::I2c(I2cResponse::RunTransaction {
                    transaction: resps,
                }))
            }
        }
    }
}

#[typetag::serde]
impl Request for EmuRequest {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError> {
        let instance = transport.emulator()?;
        match self {
            EmuRequest::GetState => Ok(Response::Emu(EmuResponse::GetState {
                state: instance.get_state()?,
            })),
            EmuRequest::Start {
                factory_reset,
                args,
            } => {
                instance.start(*factory_reset, args)?;
                Ok(Response::Emu(EmuResponse::Start))
            }
            EmuRequest::Stop => {
                instance.stop()?;
                Ok(Response::Emu(EmuResponse::Stop))
            }
        }
    }
}

#[typetag::serde]
impl Request for ProxyRequest {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError> {
        match self {
            ProxyRequest::Bootstrap { options, payload } => {
                Bootstrap::update(transport, options, payload)?;
                Ok(Response::Proxy(ProxyResponse::Bootstrap))
            }
        }
    }
}

impl<'a> TransportCommandHandler<'a> {
    pub fn new(transport: &'a TransportWrapper) -> Self {
        Self { transport }
    }

    /// This method will perform whatever action on the underlying `Transport` that is requested
    /// by the given `Request`, and return a response to be sent to the client.  Any `Err`
    /// return from this method will be propagated to the remote client, without any server-side
    /// logging.
    fn do_execute_cmd(&self, req: &dyn Request) -> Result<Response, TransportError> {
        req.handle(self.transport)
    }
}

impl<'a> CommandHandler<Message> for TransportCommandHandler<'a> {
    /// This method will perform whatever action on the underlying `Transport` that is requested
    /// by the given `Message`, and return a response to be sent to the client.  Any `Err`
    /// return from this method will be treated as an irrecoverable protocol error, causing an
    /// error message in the server log, and the connection to be terminated.
    fn execute_cmd(&self, msg: &Message) -> Result<Message> {
        if let Message::Req(req) = msg {
            // Package either `Ok()` or `Err()` into a `Message`, to be sent via network.
            let req: &dyn Request = req.borrow();
            return Ok(Message::Res(self.do_execute_cmd(req)));
        }
        bail!("Client sent non-Request to server!!!");
    }
}
