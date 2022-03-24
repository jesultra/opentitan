// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

use crate::app::TransportWrapper;
use crate::bootstrap::BootstrapOptions;
use crate::io::emu::{EmuState, EmuValue};
use crate::io::gpio::{PinMode, PullMode};
use crate::io::spi::TransferMode;
use crate::transport::{Capabilities, TransportError};
use crate::util::voltage::Voltage;

#[derive(Serialize, Deserialize)]
pub enum Message {
    Req(Box<dyn Request>),
    Res(Result<Response, TransportError>),
}

#[typetag::serde(tag = "type")]
pub trait Request {
    fn handle(&self, transport: &TransportWrapper) -> Result<Response, TransportError>;
}

#[derive(Serialize, Deserialize)]
pub enum Response {
    GetCapabilities(Capabilities),
    Gpio(GpioResponse),
    Uart(UartResponse),
    Spi(SpiResponse),
    I2c(I2cResponse),
    Emu(EmuResponse),
    Proxy(ProxyResponse),
}

#[derive(Serialize, Deserialize)]
pub struct GetCapabilitiesReq {
}

#[derive(Serialize, Deserialize)]
pub struct GpioReq {
    pub id: String,
    pub command: GpioRequest,
}

#[derive(Serialize, Deserialize)]
pub enum GpioRequest {
    Write { logic: bool },
    Read,
    SetMode { mode: PinMode },
    SetPullMode { pull: PullMode },
}

#[derive(Serialize, Deserialize)]
pub enum GpioResponse {
    Write,
    Read { value: bool },
    SetMode,
    SetPullMode,
}

#[derive(Serialize, Deserialize)]
pub struct UartReq {
    pub id: String,
    pub command: UartRequest,
}

#[derive(Serialize, Deserialize)]
pub enum UartRequest {
    GetBaudrate,
    SetBaudrate {
        rate: u32,
    },
    Read {
        timeout_millis: Option<u32>,
        len: u32,
    },
    Write {
        data: Vec<u8>,
    },
}

#[derive(Serialize, Deserialize)]
pub enum UartResponse {
    GetBaudrate { rate: u32 },
    SetBaudrate,
    Read { data: Vec<u8> },
    Write,
}

#[derive(Serialize, Deserialize)]
pub struct SpiReq {
    pub id: String,
    pub command: SpiRequest,
}

#[derive(Serialize, Deserialize)]
pub enum SpiTransferRequest {
    Read { len: u32 },
    Write { data: Vec<u8> },
    Both { data: Vec<u8> },
}

#[derive(Serialize, Deserialize)]
pub enum SpiTransferResponse {
    Read { data: Vec<u8> },
    Write,
    Both { data: Vec<u8> },
}

#[derive(Serialize, Deserialize)]
pub enum SpiRequest {
    GetTransferMode,
    SetTransferMode {
        mode: TransferMode,
    },
    GetBitsPerWord,
    SetBitsPerWord {
        bits_per_word: u32,
    },
    GetMaxSpeed,
    SetMaxSpeed {
        value: u32,
    },
    GetMaxTransferCount,
    GetMaxChunkSize,
    SetVoltage {
        voltage: Voltage,
    },
    RunTransaction {
        transaction: Vec<SpiTransferRequest>,
    },
}

#[derive(Serialize, Deserialize)]
pub enum SpiResponse {
    GetTransferMode {
        mode: TransferMode,
    },
    SetTransferMode,
    GetBitsPerWord {
        bits_per_word: u32,
    },
    SetBitsPerWord,
    GetMaxSpeed {
        speed: u32,
    },
    SetMaxSpeed,
    GetMaxTransferCount {
        number: usize,
    },
    GetMaxChunkSize {
        size: usize,
    },
    SetVoltage,
    RunTransaction {
        transaction: Vec<SpiTransferResponse>,
    },
}

#[derive(Serialize, Deserialize)]
pub struct I2cReq {
    pub id: String,
    pub command: I2cRequest,
}

#[derive(Serialize, Deserialize)]
pub enum I2cTransferRequest {
    Read { len: u32 },
    Write { data: Vec<u8> },
}

#[derive(Serialize, Deserialize)]
pub enum I2cTransferResponse {
    Read { data: Vec<u8> },
    Write,
}

#[derive(Serialize, Deserialize)]
pub enum I2cRequest {
    RunTransaction {
        address: u8,
        transaction: Vec<I2cTransferRequest>,
    },
}

#[derive(Serialize, Deserialize)]
pub enum I2cResponse {
    RunTransaction {
        transaction: Vec<I2cTransferResponse>,
    },
}
#[derive(Serialize, Deserialize)]
pub enum EmuRequest {
    GetState,
    Start {
        factory_reset: bool,
        args: HashMap<String, EmuValue>,
    },
    Stop,
}

#[derive(Serialize, Deserialize)]
pub enum EmuResponse {
    GetState { state: EmuState },
    Start,
    Stop,
}

#[derive(Serialize, Deserialize)]
pub enum ProxyRequest {
    Bootstrap {
        options: BootstrapOptions,
        payload: Vec<u8>,
    },
}

#[derive(Serialize, Deserialize)]
pub enum ProxyResponse {
    Bootstrap,
}
