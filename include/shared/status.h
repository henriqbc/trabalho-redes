#pragma once

typedef enum STATUS {
  STATUS_SUCCESS,
  STATUS_ERROR,

  STATUS_NICKNAME_TAKEN,
  STATUS_FAILURE_CREATING_SOCKET,
} STATUS;