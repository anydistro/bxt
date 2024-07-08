/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faArrowsLeftRight } from "@fortawesome/free-solid-svg-icons";
import React, { HTMLProps, useState } from "react";

type DateRangeInputProps = HTMLProps<HTMLDivElement> & {
    since: Date;
    until: Date;
    onDateChange: (since: Date, until: Date) => void;
};
const className =
    "text-base-content text-sm border-none input join-item rounded-xl px-1 focus:-outline-offset-1 focus:outline-[2.5px] text-inherit";
const DateRangeInput: React.FC<DateRangeInputProps> = ({
    since: from,
    until: to,
    onDateChange,
    ...props
}) => {
    const [fromValue, setFromValue] = useState(from.toISOString().slice(0, 16));
    const [toValue, setToValue] = useState(to.toISOString().slice(0, 16));

    const handleFromChange = (event: React.ChangeEvent<HTMLInputElement>) => {
        const newFromValue = event.target.value;
        setFromValue(newFromValue);
        onDateChange(new Date(newFromValue), new Date(toValue));
    };

    const handleToChange = (event: React.ChangeEvent<HTMLInputElement>) => {
        const newToValue = event.target.value;
        setToValue(newToValue);
        onDateChange(new Date(fromValue), new Date(newToValue));
    };

    return (
        <div className="flex items-center gap-2" {...props}>
            <input
                className={className}
                aria-label="From"
                type="datetime-local"
                value={fromValue}
                onChange={handleFromChange}
                max={toValue}
            />
            <FontAwesomeIcon
                className="text-base-content w-10 text-inherit"
                icon={faArrowsLeftRight}
            />
            <input
                className={className}
                aria-label="To"
                type="datetime-local"
                value={toValue}
                onChange={handleToChange}
                min={fromValue}
            />
        </div>
    );
};

export default DateRangeInput;
