// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

use serde::{Deserialize, Serialize};

use crate::io::gpio::{PinMode, PullMode, GpioError};
use crate::io::i2c::I2cError;
use crate::io::spi::{TransferMode, SpiError};
use crate::util::voltage::Voltage;

#[derive(Serialize, Deserialize)]
pub enum ControlPacket {
    Req(Request),
    Res(Result<Response, TransportError>)
}

#[derive(Serialize, Deserialize)]
pub enum Request {
    Gpio(GpioRequest),
    Uart(UartRequest),
    Spi(SpiRequest),
    I2c(I2cRequest)
}

#[derive(Serialize, Deserialize)]
pub enum Response {
    Gpio(GpioResponse),
    Uart(UartResponse),
    Spi(SpiResponse),
    I2c(I2cResponse)
}

#[derive(Serialize, Deserialize)]
pub enum TransportError {
    Gpio(GpioError),
    Spi(SpiError),
    I2c(I2cError),
}

#[derive(Serialize, Deserialize)]
pub enum GpioRequest {
    Write { id: String, logic: bool },
    Read { id: String },
    SetMode { id: String, mode: PinMode },
    SetPullMode { id: String, pull: PullMode },
}

#[derive(Serialize, Deserialize)]
pub enum GpioResponse {
    Write,
    Read {
        value: bool
    },
    SetMode,
    SetPullMode,
}

#[derive(Serialize, Deserialize)]
pub enum UartRequest {
    GetBaudrate,
    SetBaudrate{
        rate: u32
    },
    Read {
        timeout: Option<u32>,
        len: u32,
    },
    Write {
        data: Vec<u8>,
    },
}

#[derive(Serialize, Deserialize)]
pub enum UartResponse {
    GetBaudrate {
        rate: u32,
    },
    SetBaudrate,
    Read {
        data: Vec<u8>,
    },
    Write {
        len: u32,
    }
}

#[derive(Serialize, Deserialize)]
pub enum SpiTransferRequest {
    Read {
        len: u32
    },
    Write {
        data: Vec<u8>
    },
    Both {
        read_len: u32,
        write_data: Vec<u8>,
    }
}

#[derive(Serialize, Deserialize)]
pub enum SpiTransferResponse {
    Read {
        data: Vec<u8>,
    },
    Write {
        len: u32,
    },
    Both {
        read_data: Vec<u8>,
        write_len: u32,
    }
}

#[derive(Serialize, Deserialize)]
pub enum SpiRequest {
    GetTransferMode,
    SetTransferMode {
        mode: TransferMode
    },
    GetBitsPerWord,
    SetBitsPerWord {
        bits_per_word: u32,
    },
    GetMaxSpeed,
    SetMaxSpeed{
        value: u32
    },
    GetMaxTransferCount,
    MaxChunkSize,
    SetVoltage {
        voltage: Voltage,
    },
    Transfer(SpiTransferRequest)
}

#[derive(Serialize, Deserialize)]
pub enum SpiResponse {
    GetTransferMode {
        mode: TransferMode
    },
    SetTransferMode,
    GetBitsPerWord {
        bits_per_word: u32,
    },
    SetBitsPerWord,
    GetMaxSpeed{
        speed: u32
    },
    SetMaxSpeed,
    GetMaxTransferCount {
        number: usize
    },
    MaxChunkSize {
        size: usize
    },
    SetVoltage{
        voltage: Voltage,
    },
    Transfer(SpiTransferResponse)
}

#[derive(Serialize, Deserialize)]
pub enum I2cRequest {
    Read {
        address: u8,
        len: u32,
    },
    Write {
        address: u8,
        data: Vec<u8>,
    }
}

#[derive(Serialize, Deserialize)]
pub enum I2cResponse {
    Read {
        address: u8,
        data: Vec<u8>,
    },
    Write {
        address: u8,
        len: u32,
    }
}

